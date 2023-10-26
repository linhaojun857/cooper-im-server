#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <sw/redis++/redis++.h>

#include <cooper/net/TcpConnection.hpp>
#include <unordered_map>

#include "entity/Entity.hpp"

using namespace cooper;
using namespace sw::redis;

class IMStore {
public:
    static IMStore* getInstance();

    IMStore();

    void setRedisConn(const std::shared_ptr<Redis>& redisConn);

    std::shared_ptr<Redis> getRedisConn();

    void addOnlineUser(int id, const User& user);

    std::shared_ptr<User> getOnlineUser(int id);

    void removeOnlineUser(int id);

    bool isOnlineUser(int id);

    void addTcpConnection(int id, const TcpConnectionPtr& connPtr);

    TcpConnectionPtr getTcpConnection(int id);

    void removeTcpConnectionById(int id);

    void removeTcpConnectionByConn(const TcpConnectionPtr& connPtr);

    bool haveTcpConnection(int id);

private:
    std::shared_ptr<Redis> redisConn_;
    std::unordered_map<int, TcpConnectionPtr> tcpConnections_;
    std::unordered_map<TcpConnectionPtr, int> tcpConnectionsReverse_;
};

#endif
