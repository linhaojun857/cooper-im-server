#include "MsgController.hpp"

#include <algorithm>
#include <cooper/util/Logger.hpp>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"
#include "util/JwtUtil.hpp"

MsgController::MsgController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
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
    auto user = IMStore::getInstance()->getOnlineUser(userId);
    if (personMessage.from_id != user->id) {
        RETURN_ERROR("from_id与token不匹配")
    }
    GET_SQL_CONN_T(sqlConn)
    std::vector<int> friendIds;
    auto temps = sqlConn->query<std::tuple<int>>("select b_id from t_friend where a_id =" + std::to_string(userId));
    for (const auto& temp : temps) {
        friendIds.emplace_back(std::get<0>(temp));
    }
    if (std::find(friendIds.begin(), friendIds.end(), personMessage.to_id) == friendIds.end()) {
        RETURN_ERROR("对方不是你的好友")
    }
    try {
        auto ret = sqlConn->query<std::tuple<std::string>>(
            "select session_id from t_friend where a_id = " + std::to_string(personMessage.from_id) +
            " and b_id = " + std::to_string(personMessage.to_id));
        personMessage.session_id = std::get<0>(ret[0]);
        personMessage.timestamp = time(nullptr);
        sqlConn->insert(personMessage);
        auto msg_id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()")[0];
        personMessage.id = std::get<0>(msg_id);
    } catch (const std::exception& e) {
        LOG_ERROR << e.what();
        RETURN_ERROR("发送失败")
    }
    auto j1 = personMessage.toJson();
    j1["type"] = PROTOCOL_TYPE_PERSON_MESSAGE_SEND;
    j1["status"] = SYNC_DATA_PERSON_MESSAGE_INSERT;
    connPtr->sendJson(j1);
    if (IMStore::getInstance()->haveTcpConnection(personMessage.to_id)) {
        auto toConnPtr = IMStore::getInstance()->getTcpConnection(personMessage.to_id);
        j1["type"] = PROTOCOL_TYPE_PERSON_MESSAGE_RECV;
        j1["status"] = SYNC_DATA_PERSON_MESSAGE_INSERT;
        toConnPtr->sendJson(j1);
    } else {
        redisConn_->lpush(REDIS_KEY_PERSON_OFFLINE_MSG + std::to_string(personMessage.to_id),
                          personMessage.toJson().dump());
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
    GET_SQL_CONN_H(sqlConn)
    auto pms = sqlConn->query<PersonMessage>(
        "select * "
        "from t_person_message "
        "where session_id in (select session_id from t_friend where a_id = " +
        std::to_string(userId) + ")");
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
    auto ret = redisConn_->lpop(REDIS_KEY_PERSON_OFFLINE_MSG + std::to_string(userId));
    while (ret.has_value()) {
        j["personMessages"].push_back(json::parse(ret.value()));
        ret = redisConn_->lpop(REDIS_KEY_PERSON_OFFLINE_MSG + std::to_string(userId));
    }
    if (!j.contains("personMessages")) {
        LOG_DEBUG << "没有离线消息";
    }
    LOG_DEBUG << "json:" << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}

void MsgController::handleGroupSendMsg(const cooper::TcpConnectionPtr& connPtr, const nlohmann::json& params) {
    LOG_DEBUG << "MsgController::handleGroupSendMsg";
    TCP_CHECK_PARAMS(params, "token", "groupMessage")
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("token无效")
    }
    const auto& gmJson = params["groupMessage"];
    TCP_CHECK_PARAMS(gmJson, "from_id", "group_id", "message_type", "message", "file_url")
    auto groupMessage = GroupMessage::fromJson(gmJson);
    auto user = IMStore::getInstance()->getOnlineUser(userId);
    if (groupMessage.from_id != user->id) {
        RETURN_ERROR("from_id与token不匹配")
    }
    GET_SQL_CONN_T(sqlConn)
    std::vector<int> groupMemberIds;
    auto temps = sqlConn->query<std::tuple<int>>("select user_id from t_user_group where group_id =" +
                                                 std::to_string(groupMessage.group_id));
    for (const auto& temp : temps) {
        groupMemberIds.emplace_back(std::get<0>(temp));
    }
    if (std::find(groupMemberIds.begin(), groupMemberIds.end(), groupMessage.from_id) == groupMemberIds.end()) {
        RETURN_ERROR("你不在该群组中")
    }
    try {
        groupMessage.timestamp = time(nullptr);
        sqlConn->insert(groupMessage);
        auto msg_id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()")[0];
        groupMessage.id = std::get<0>(msg_id);
    } catch (const std::exception& e) {
        LOG_ERROR << e.what();
        RETURN_ERROR("发送失败")
    }
    GroupMessageDTO gmd(groupMessage);
    gmd.from_nickname = user->nickname;
    gmd.from_avatar = user->avatar;
    auto j1 = gmd.toJson();
    j1["type"] = PROTOCOL_TYPE_GROUP_MESSAGE_SEND;
    j1["status"] = SYNC_DATA_GROUP_MESSAGE_INSERT;
    connPtr->sendJson(j1);
    j1["type"] = PROTOCOL_TYPE_GROUP_MESSAGE_RECV;
    j1["status"] = SYNC_DATA_GROUP_MESSAGE_INSERT;
    for (const auto& memberId : groupMemberIds) {
        if (memberId == userId) {
            continue;
        }
        if (IMStore::getInstance()->haveTcpConnection(memberId)) {
            auto toConnPtr = IMStore::getInstance()->getTcpConnection(memberId);
            toConnPtr->sendJson(j1);
        } else {
            redisConn_->lpush(REDIS_KEY_GROUP_OFFLINE_MSG + std::to_string(memberId), j1.dump());
            auto ret = redisConn_->get(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(memberId));
            if (!ret.has_value()) {
                SyncState syncState(memberId);
                syncState.group_message_sync_state = 1;
                syncState.addInsertedGroupMessageId(groupMessage.id);
                redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(memberId), syncState.toJson().dump());
            } else {
                SyncState syncState = SyncState::fromJson(json::parse(ret.value()));
                syncState.group_message_sync_state = 1;
                syncState.addInsertedGroupMessageId(groupMessage.id);
                redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(memberId), syncState.toJson().dump());
            }
        }
    }
}

void MsgController::getAllGroupMessages(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "MsgController::getAllGroupMessages";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token");
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    auto gms = sqlConn->query<GroupMessageDTO>(
        "select a.*, b.nickname as from_nickname, b.avatar as from_avatar "
        "from t_group_message a, t_user b "
        "where a.from_id = b.id and a.group_id in "
        "(select group_id from t_user_group where user_id = " +
        std::to_string(userId) + ")");
    for (auto& gm : gms) {
        j["groupMessages"].push_back(gm.toJson());
    }
    LOG_DEBUG << "json:" << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}

void MsgController::getSyncGroupMessages(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "MsgController::getSyncGroupMessages";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token")
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto ret = redisConn_->lpop(REDIS_KEY_GROUP_OFFLINE_MSG + std::to_string(userId));
    while (ret.has_value()) {
        j["groupMessages"].push_back(json::parse(ret.value()));
        ret = redisConn_->lpop(REDIS_KEY_GROUP_OFFLINE_MSG + std::to_string(userId));
    }
    if (!j.contains("groupMessages")) {
        LOG_DEBUG << "没有离线消息";
    }
    LOG_DEBUG << "json:" << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}
