#include "MsgController.hpp"

#include <algorithm>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

MsgController::MsgController(std::shared_ptr<dbng<mysql>> sqlConn, std::shared_ptr<Redis> redisConn)
    : sqlConn_(std::move(sqlConn)), redisConn_(std::move(redisConn)) {
}

void MsgController::handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params) {
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
    sqlConn_->insert(personMessage);
    if (IMStore::getInstance()->haveTcpConnection(personMessage.to_id)) {
        auto toConnPtr = IMStore::getInstance()->getTcpConnection(personMessage.to_id);
        toConnPtr->send(personMessage.toJson().dump());
    } else {
        redisConn_->lpush(REDIS_KEY_OFFLINE_MSG + std::to_string(personMessage.to_id), personMessage.toJson().dump());
    }
}
