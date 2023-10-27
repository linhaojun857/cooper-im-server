#include "MsgController.hpp"

#include <algorithm>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

MsgController::MsgController(std::shared_ptr<dbng<mysql>> sqlConn) : sqlConn_(std::move(sqlConn)) {
}

void MsgController::handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params) {
    TCP_CHECK_PARAMS(params, "token", "personMessage")
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("token无效")
    }
    const auto& pmJson = params["personMessage"];
    TCP_CHECK_PARAMS(pmJson, "from_id", "to_id", "msg_type", "msg", "file_url")
    auto personMessage = PersonMessage::fromJson(pmJson);
    std::shared_ptr<User> user = IMStore::getInstance()->getOnlineUser(userId);
    if (personMessage.from_id != user->id) {
        RETURN_ERROR("from_id与token不匹配")
    }
    std::vector<int> friendIds;
    auto friends = sqlConn_->query<Friend>("a_id = ?", userId);
    for (const auto& fri : friends) {
        friendIds.emplace_back(fri.b_id);
    }
    if (std::find(friendIds.begin(), friendIds.end(), personMessage.to_id) == friendIds.end()) {
        RETURN_ERROR("对方不是你的好友")
    }
    personMessage.timestamp = time(nullptr);
    sqlConn_->insert(personMessage);
    if (IMStore::getInstance()->haveTcpConnection(personMessage.to_id)) {
        auto toConnPtr = IMStore::getInstance()->getTcpConnection(personMessage.to_id);
        toConnPtr->send(personMessage.toJson().dump());
    } else {
        auto redisConn = IMStore::getInstance()->getRedisConn();
        redisConn->lpush(REDIS_KEY_OFFLINE_MSG + std::to_string(personMessage.to_id), personMessage.toJson().dump());
    }
}
