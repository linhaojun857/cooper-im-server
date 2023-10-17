#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <unordered_set>

class IMStore {
public:
    static std::unordered_set<int> onlineUsers;

    static void addOnlineUser(int id);

    static void removeOnlineUser(int id);

    static bool isOnlineUser(int id);
};

#endif
