#include "IMStore.hpp"

std::unordered_set<int> IMStore::onlineUsers;

void IMStore::addOnlineUser(int id) {
    onlineUsers.insert(id);
}

void IMStore::removeOnlineUser(int id) {
    onlineUsers.erase(id);
}

bool IMStore::isOnlineUser(int id) {
    return onlineUsers.find(id) != onlineUsers.end();
}
