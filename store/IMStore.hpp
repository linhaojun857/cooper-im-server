#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <unordered_map>

#include "entity/Entity.hpp"

class IMStore {
public:
    static IMStore* getInstance();

    IMStore();

    std::unordered_map<int, User> onlineUsers;

    void addOnlineUser(int id, const User& user);

    User getOnlineUser(int id);

    void removeOnlineUser(int id);

    bool isOnlineUser(int id);
};

#endif
