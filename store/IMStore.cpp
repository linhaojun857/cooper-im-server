#include "IMStore.hpp"

#include "define/IMDefine.hpp"

IMStore* IMStore::getInstance() {
    static IMStore store;
    return &store;
}

IMStore::IMStore() = default;

void IMStore::setRedisConn(const std::shared_ptr<Redis>& redisConn) {
    redisConn_ = redisConn;
}

std::shared_ptr<Redis> IMStore::getRedisConn() {
    return redisConn_;
}

void IMStore::addOnlineUser(int id, const User& user) {
    redisConn_->hset(REDIS_KEY_ONLINE_USERS, std::to_string(id), user.toJson().dump());
}

std::shared_ptr<User> IMStore::getOnlineUser(int id) {
    auto json = redisConn_->hget(REDIS_KEY_ONLINE_USERS, std::to_string(id));
    return User::fromJson(json::parse(json.value()));
}

void IMStore::removeOnlineUser(int id) {
    redisConn_->hdel(REDIS_KEY_ONLINE_USERS, std::to_string(id));
}

bool IMStore::isOnlineUser(int id) {
    return redisConn_->hexists(REDIS_KEY_ONLINE_USERS, std::to_string(id));
}

void IMStore::addBusinessTcpConnection(int id, const TcpConnectionPtr& connPtr) {
    businessTcpConnections_[id] = connPtr;
    businessTcpConnectionsReverse_[connPtr] = id;
}

TcpConnectionPtr IMStore::getBusinessTcpConnection(int id) {
    return businessTcpConnections_[id];
}

void IMStore::removeBusinessTcpConnectionById(int id) {
    removeOnlineUser(id);
    businessTcpConnectionsReverse_.erase(businessTcpConnections_[id]);
    businessTcpConnections_.erase(id);
}

void IMStore::removeBusinessTcpConnectionByConn(const TcpConnectionPtr& connPtr) {
    int userId = businessTcpConnectionsReverse_[connPtr];
    auto ret = redisConn_->get(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId));
    if (ret.has_value()) {
        int roomId = std::stoi(ret.value());
        redisConn_->del(REDIS_KEY_LIVE_ROOM + std::to_string(roomId));
        redisConn_->srem(REDIS_KEY_LIVE_ROOM_SET, std::to_string(roomId));
        redisConn_->del(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId));
        redisConn_->del(REDIS_KEY_LIVE_ROOM_VIEWER_SET + std::to_string(roomId));
        liveController_->notifyUsersWhenLiveEnd(roomId);
    }
    removeOnlineUser(userId);
    businessTcpConnections_.erase(userId);
    businessTcpConnectionsReverse_.erase(connPtr);
}

bool IMStore::haveBusinessTcpConnection(int id) {
    return businessTcpConnections_.find(id) != businessTcpConnections_.end();
}

void IMStore::addMediaTcpConnection(int id, const TcpConnectionPtr& connPtr) {
    mediaTcpConnections_[id] = connPtr;
    mediaTcpConnectionsReverse_[connPtr] = id;
}

TcpConnectionPtr IMStore::getMediaTcpConnection(int id) {
    return mediaTcpConnections_[id];
}

void IMStore::removeMediaTcpConnectionById(int id) {
    mediaTcpConnectionsReverse_.erase(mediaTcpConnections_[id]);
    mediaTcpConnections_.erase(id);
}

void IMStore::removeMediaTcpConnectionByConn(const TcpConnectionPtr& connPtr) {
    int userId = mediaTcpConnectionsReverse_[connPtr];
    auto ret = redisConn_->get(REDIS_KEY_VIDEO_CALL + std::to_string(userId));
    if (ret.has_value()) {
        int peerId = std::stoi(ret.value());
        redisConn_->del(REDIS_KEY_VIDEO_CALL + std::to_string(userId));
        redisConn_->del(REDIS_KEY_VIDEO_CALL + std::to_string(peerId));
        if (IMStore::getInstance()->haveBusinessTcpConnection(peerId)) {
            auto toConnPtr = IMStore::getInstance()->getBusinessTcpConnection(peerId);
            json j;
            j["type"] = PROTOCOL_TYPE_VIDEO_CALL_END;
            j["from_id"] = userId;
            j["to_id"] = peerId;
            toConnPtr->sendJson(j);
        }
    }
    mediaTcpConnections_.erase(userId);
    mediaTcpConnectionsReverse_.erase(connPtr);
}

bool IMStore::haveMediaTcpConnection(int id) {
    return mediaTcpConnections_.find(id) != mediaTcpConnections_.end();
}

void IMStore::registerFileController(FileController* fileController) {
    fileController_ = fileController;
}

void IMStore::registerFriendController(FriendController* friendController) {
    friendController_ = friendController;
}

void IMStore::registerGroupController(GroupController* groupController) {
    groupController_ = groupController;
}

void IMStore::registerLiveController(LiveController* liveController) {
    liveController_ = liveController;
}

void IMStore::registerMsgController(MsgController* msgController) {
    msgController_ = msgController;
}

void IMStore::registerUserController(UserController* userController) {
    userController_ = userController;
}

void IMStore::registerAVCallController(AVCallController* avCallController) {
    avCallController_ = avCallController;
}
