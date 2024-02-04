#ifndef store_IMStore_hpp
#define store_IMStore_hpp

#include <sw/redis++/redis++.h>

#include <cooper/net/TcpConnection.hpp>
#include <unordered_map>

#include "controller/AVCallController.hpp"
#include "controller/FileController.hpp"
#include "controller/FriendController.hpp"
#include "controller/GroupController.hpp"
#include "controller/LiveController.hpp"
#include "controller/MsgController.hpp"
#include "controller/PyqController.hpp"
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

    void addBusinessTcpConnection(int id, const TcpConnectionPtr& connPtr);

    TcpConnectionPtr getBusinessTcpConnection(int id);

    void removeBusinessTcpConnectionById(int id);

    void removeBusinessTcpConnectionByConn(const TcpConnectionPtr& connPtr);

    bool haveBusinessTcpConnection(int id);

    void addMediaTcpConnection(int id, const TcpConnectionPtr& connPtr);

    TcpConnectionPtr getMediaTcpConnection(int id);

    void removeMediaTcpConnectionById(int id);

    void removeMediaTcpConnectionByConn(const TcpConnectionPtr& connPtr);

    bool haveMediaTcpConnection(int id);

    void registerFileController(FileController* fileController);

    void registerFriendController(FriendController* friendController);

    void registerGroupController(GroupController* groupController);

    void registerLiveController(LiveController* liveController);

    void registerMsgController(MsgController* msgController);

    void registerUserController(UserController* userController);

    void registerAVCallController(AVCallController* avCallController);

    void registerPyqController(PyqController* pyqController);

private:
    std::shared_ptr<Redis> redisConn_;
    std::unordered_map<int, TcpConnectionPtr> businessTcpConnections_;
    std::unordered_map<TcpConnectionPtr, int> businessTcpConnectionsReverse_;
    std::unordered_map<int, TcpConnectionPtr> mediaTcpConnections_;
    std::unordered_map<TcpConnectionPtr, int> mediaTcpConnectionsReverse_;
    FileController* fileController_ = nullptr;
    FriendController* friendController_ = nullptr;
    GroupController* groupController_ = nullptr;
    LiveController* liveController_ = nullptr;
    MsgController* msgController_ = nullptr;
    UserController* userController_ = nullptr;
    AVCallController* avCallController_ = nullptr;
    PyqController* pyqController_ = nullptr;
};

#endif
