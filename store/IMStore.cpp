#include "IMStore.hpp"

#include <utility>

IMStore* IMStore::getInstance() {
    static IMStore store;
    return &store;
}

IMStore::IMStore() = default;

void IMStore::addOnlineUser(int id, const User& user) {
    onlineUsers[id] = user;
}

User IMStore::getOnlineUser(int id) {
    return onlineUsers[id];
}

void IMStore::removeOnlineUser(int id) {
    onlineUsers.erase(id);
}

bool IMStore::isOnlineUser(int id) {
    return onlineUsers.find(id) != onlineUsers.end();
}
