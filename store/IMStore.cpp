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

void IMStore::addTcpConnection(int id, const cooper::TcpConnectionPtr& connPtr) {
    tcpConnections_[id] = connPtr;
    tcpConnectionsReverse_[connPtr] = id;
}

TcpConnectionPtr IMStore::getTcpConnection(int id) {
    return tcpConnections_[id];
}

void IMStore::removeTcpConnectionById(int id) {
    removeOnlineUser(id);
    tcpConnectionsReverse_.erase(tcpConnections_[id]);
    tcpConnections_.erase(id);
}

void IMStore::removeTcpConnectionByConn(const cooper::TcpConnectionPtr& connPtr) {
    int userId = tcpConnectionsReverse_[connPtr];
    auto ret = redisConn_->get(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId));
    if (ret.has_value()) {
        int roomId = std::stoi(ret.value());
        redisConn_->del(REDIS_KEY_LIVE_ROOM + std::to_string(roomId));
        redisConn_->srem(REDIS_KEY_LIVE_ROOM_SET, std::to_string(roomId));
        redisConn_->del(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId));
        liveController_->notifyUsersWhenLiveEnd(roomId);
    }
    removeOnlineUser(userId);
    tcpConnections_.erase(userId);
    tcpConnectionsReverse_.erase(connPtr);
}

bool IMStore::haveTcpConnection(int id) {
    return tcpConnections_.find(id) != tcpConnections_.end();
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
