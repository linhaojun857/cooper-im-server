#include "IMStore.hpp"

#include <utility>

IMStore* IMStore::getInstance() {
    static IMStore store;
    return &store;
}

IMStore::IMStore() = default;

void IMStore::addOnlineUser(int id, const User& user) {
    onlineUsers_[id] = user;
}

User IMStore::getOnlineUser(int id) {
    return onlineUsers_[id];
}

void IMStore::removeOnlineUser(int id) {
    onlineUsers_.erase(id);
}

bool IMStore::isOnlineUser(int id) {
    return onlineUsers_.find(id) != onlineUsers_.end();
}

void IMStore::addTcpConnection(int id, const cooper::TcpConnectionPtr& connPtr) {
    tcpConnections_[id] = connPtr;
}

TcpConnectionPtr IMStore::getTcpConnection(int id) {
    return tcpConnections_[id];
}

void IMStore::removeTcpConnection(int id) {
    tcpConnections_.erase(id);
}

bool IMStore::haveTcpConnection(int id) {
    return tcpConnections_.find(id) != tcpConnections_.end();
}
