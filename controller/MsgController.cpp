#include "MsgController.hpp"

#include <algorithm>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

MsgController::MsgController(std::shared_ptr<dbng<mysql>> sqlConn) : sqlConn_(std::move(sqlConn)) {
}

void MsgController::handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params) {
    TCP_CHECK_PARAMS(params, "from_id", "to_id", "msg_type", "msg", "file_url", "token")
    auto from_id = params["from_id"].get<int>();
    auto to_id = params["to_id"].get<int>();
    auto msg_type = params["msg_type"].get<int>();
    auto msg = params["msg"].get<std::string>();
    auto file_url = params["file_url"].get<std::string>();
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("token无效")
    }
    std::shared_ptr<User> user = IMStore::getInstance()->getOnlineUser(userId);
    if (from_id != user->id) {
        RETURN_ERROR("from_id与token不匹配")
    }
    std::vector<int> friendIds;
    auto friends = sqlConn_->query<Friend>("a_id = ?", userId);
    for (const auto& fri : friends) {
        friendIds.emplace_back(fri.b_id);
    }
    if (std::find(friendIds.begin(), friendIds.end(), to_id) == friendIds.end()) {
        RETURN_ERROR("对方不是你的好友")
    }
    PersonMessage pm(from_id, to_id, msg_type, msg, file_url, time(nullptr));
    sqlConn_->insert(pm);
    if (IMStore::getInstance()->haveTcpConnection(to_id)) {
        auto toConnPtr = IMStore::getInstance()->getTcpConnection(to_id);
        toConnPtr->send(pm.toJson().dump());
    } else {
        auto redisConn = IMStore::getInstance()->getRedisConn();
        redisConn->lpush(REDIS_KEY_OFFLINE_MSG + std::to_string(to_id), pm.toJson().dump());
    }
}
