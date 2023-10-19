#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <unordered_map>

#include "entity/Entity.hpp"

class IMStore {
public:
    static std::unordered_map<int, User> onlineUsers;

    static void addOnlineUser(int id, User user);

    static void removeOnlineUser(int id);

    static bool isOnlineUser(int id);
};

#endif
