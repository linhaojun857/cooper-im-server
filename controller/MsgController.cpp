#include "MsgController.hpp"

#include <algorithm>
#include <cooper/util/Logger.hpp>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

MsgController::MsgController(std::shared_ptr<dbng<mysql>> sqlConn, std::shared_ptr<Redis> redisConn)
    : sqlConn_(std::move(sqlConn)), redisConn_(std::move(redisConn)) {
}

void MsgController::handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params) {
    LOG_DEBUG << "MsgController::handlePersonSendMsg";
    TCP_CHECK_PARAMS(params, "token", "personMessage")
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("token无效")
    }
    const auto& pmJson = params["personMessage"];
    TCP_CHECK_PARAMS(pmJson, "from_id", "to_id", "message_type", "message", "file_url")
    auto personMessage = PersonMessage::fromJson(pmJson);
    std::shared_ptr<User> user = IMStore::getInstance()->getOnlineUser(userId);
    if (personMessage.from_id != user->id) {
        RETURN_ERROR("from_id与token不匹配")
    }
    std::vector<int> friendIds;
    auto temps = sqlConn_->query<std::tuple<int>>("select b_id from friend where a_id = ?", userId);
    for (const auto& temp : temps) {
        friendIds.emplace_back(std::get<0>(temp));
    }
    if (std::find(friendIds.begin(), friendIds.end(), personMessage.to_id) == friendIds.end()) {
        RETURN_ERROR("对方不是你的好友")
    }
    personMessage.timestamp = time(nullptr);
    try {
        sqlConn_->insert(personMessage);
        auto msg_id = sqlConn_->query<std::tuple<int>>("select LAST_INSERT_ID()")[0];
        personMessage.id = std::get<0>(msg_id);
    } catch (const std::exception& e) {
        LOG_ERROR << e.what();
        RETURN_ERROR("发送失败")
    }
    if (IMStore::getInstance()->haveTcpConnection(personMessage.to_id)) {
        auto toConnPtr = IMStore::getInstance()->getTcpConnection(personMessage.to_id);
        auto j1 = personMessage.toJson();
        j1["type"] = PROTOCOL_TYPE_PERSON_MESSAGE_RECV;
        j1["status"] = SYNC_DATA_PERSON_MESSAGE_INSERT;
        toConnPtr->sendJson(j1);
    } else {
        redisConn_->lpush(REDIS_KEY_OFFLINE_MSG + std::to_string(personMessage.to_id), personMessage.toJson().dump());
        auto ret = redisConn_->get(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(personMessage.to_id));
        if (!ret.has_value()) {
            SyncState syncState(personMessage.to_id);
            syncState.person_message_sync_state = 1;
            syncState.addInsertedPersonMessageId(personMessage.id);
            redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(personMessage.to_id),
                            syncState.toJson().dump());
        } else {
            SyncState syncState = SyncState::fromJson(json::parse(ret.value()));
            syncState.person_message_sync_state = 1;
            syncState.addInsertedPersonMessageId(personMessage.id);
            redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(personMessage.to_id),
                            syncState.toJson().dump());
        }
    }
}

void MsgController::getAllPersonMessages(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "MsgController::getAllPersonMessages";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token");
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto pms =
        sqlConn_->query<PersonMessage>("from_id = " + std::to_string(userId) + " or to_id = " + std::to_string(userId));
    for (auto& pm : pms) {
        j["personMessages"].push_back(pm.toJson());
    }
    LOG_DEBUG << "json:" << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}

void MsgController::getSyncPersonMessages(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "MsgController::getSyncPersonMessages";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token")
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto ret = redisConn_->lpop(REDIS_KEY_OFFLINE_MSG + std::to_string(userId));
    while (!ret.has_value()) {
        j["personMessages"].push_back(json::parse(ret.value()));
        ret = redisConn_->lpop(REDIS_KEY_OFFLINE_MSG + std::to_string(userId));
    }
    if (!j.contains("personMessages")) {
        LOG_DEBUG << "没有离线消息";
    }
    LOG_DEBUG << "json:" << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}
