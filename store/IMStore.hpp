#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <sw/redis++/redis++.h>

#include <cooper/net/TcpConnection.hpp>
#include <unordered_map>

#include "controller/FileController.hpp"
#include "controller/FriendController.hpp"
#include "controller/GroupController.hpp"
#include "controller/LiveController.hpp"
#include "controller/MsgController.hpp"
#include "controller/UserController.hpp"
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

    void registerFileController(FileController* fileController);

    void registerFriendController(FriendController* friendController);

    void registerGroupController(GroupController* groupController);

    void registerLiveController(LiveController* liveController);

    void registerMsgController(MsgController* msgController);

    void registerUserController(UserController* userController);

private:
    std::shared_ptr<Redis> redisConn_;
    std::unordered_map<int, TcpConnectionPtr> tcpConnections_;
    std::unordered_map<TcpConnectionPtr, int> tcpConnectionsReverse_;
    FileController* fileController_ = nullptr;
    FriendController* friendController_ = nullptr;
    GroupController* groupController_ = nullptr;
    LiveController* liveController_ = nullptr;
    MsgController* msgController_ = nullptr;
    UserController* userController_ = nullptr;
};

#endif
