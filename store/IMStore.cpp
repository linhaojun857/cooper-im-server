#include "IMStore.hpp"

#include <utility>

std::unordered_map<int, User> IMStore::onlineUsers;

void IMStore::addOnlineUser(int id, User user) {
    onlineUsers[id] = std::move(user);
}

void IMStore::removeOnlineUser(int id) {
    onlineUsers.erase(id);
}

bool IMStore::isOnlineUser(int id) {
    return onlineUsers.find(id) != onlineUsers.end();
}
