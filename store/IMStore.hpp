#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <cooper/net/TcpConnection.hpp>
#include <unordered_map>

#include "entity/Entity.hpp"

using namespace cooper;

class IMStore {
public:
    static IMStore* getInstance();

    IMStore();

    void addOnlineUser(int id, const User& user);

    User getOnlineUser(int id);

    void removeOnlineUser(int id);

    bool isOnlineUser(int id);

    void addTcpConnection(int id, const TcpConnectionPtr& connPtr);

    TcpConnectionPtr getTcpConnection(int id);

    void removeTcpConnection(int id);

    bool haveTcpConnection(int id);

private:
    std::unordered_map<int, User> onlineUsers_;
    std::unordered_map<int, TcpConnectionPtr> tcpConnections_;
};

#endif
