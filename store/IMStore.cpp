#include "IMStore.hpp"

#include <utility>

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
    removeOnlineUser(userId);
    tcpConnections_.erase(userId);
    tcpConnectionsReverse_.erase(connPtr);
}

bool IMStore::haveTcpConnection(int id) {
    return tcpConnections_.find(id) != tcpConnections_.end();
}
