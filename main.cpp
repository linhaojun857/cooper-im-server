#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/AppTcpServer.hpp>
#include <cooper/net/HttpServer.hpp>
#include <cooper/net/TcpConnectionImpl.hpp>
#include <cooper/util/AsyncLogWriter.hpp>
#include <dbng.hpp>
#include <mysql.hpp>
#include <regex>

#include "controller/AVCallController.hpp"
#include "controller/FileController.hpp"
#include "controller/FriendController.hpp"
#include "controller/GroupController.hpp"
#include "controller/LiveController.hpp"
#include "controller/MsgController.hpp"
#include "controller/PyqController.hpp"
#include "controller/UserController.hpp"
#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"

#define ADD_HTTP_MOUNTPOINT(mountPoint, dir) \
    Headers headers;                         \
    httpServer->addMountPoint(mountPoint, dir, headers);

#define ADD_HTTP_ENDPOINT(httpMethod, path, controller, method)                                   \
    httpServer->addEndpoint(httpMethod, path, [objectPtr = &controller](auto&& PH1, auto&& PH2) { \
        objectPtr->method(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));    \
    });

#define ADD_BUSINESS_TCP_ENDPOINT(type, controller, method)                                              \
    businessTcpServer->registerBusinessHandler(type, [objectPtr = &controller](auto&& PH1, auto&& PH2) { \
        objectPtr->method(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));           \
    });

#define ADD_MEDIA_TCP_ENDPOINT(type, controller, method)                                                       \
    mediaTcpServer->registerMediaHandler(type, [objectPtr = &controller](auto&& PH1, auto&& PH2, auto&& PH3) { \
        objectPtr->method(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2),                  \
                          std::forward<decltype(PH3)>(PH3));                                                   \
    });

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;
using namespace std::placeholders;

int main() {
    AsyncLogWriter writer;
    Logger::setLogLevel(Logger::kTrace);
    Logger::setOutputFunction(
        [objectPtr = &writer](auto&& PH1, auto&& PH2) {
            objectPtr->write(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        },
        [objectPtr = &writer] {
            objectPtr->flushAll();
        });

    connection_pool<dbng<mysql>>::instance().init(MYSQL_CONNECTION_POOL_SIZE, MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME,
                                                  MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE, MYSQL_SERVER_TIMEOUT,
                                                  MYSQL_SERVER_PORT);
    auto sqlConnPool = &connection_pool<dbng<mysql>>::instance();
    {
        GET_SQL_CONN(sqlConn, sqlConnPool)
        if (!sqlConn->create_datatable<User>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<Friend>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<Notify>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<FriendApply>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<PersonMessage>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<Group>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<UserGroup>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<GroupApply>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<GroupMessage>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<File>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<LiveRoom>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<Pyq>(ormpp_auto_key{"id"}) ||
            !sqlConn->create_datatable<PyqComment>(ormpp_auto_key{"id"})) {
            LOG_ERROR << "create table failed";
            return -1;
        }
    }

    ConnectionOptions connectionOptions;
    connectionOptions.host = REDIS_SERVER_IP;
    connectionOptions.port = REDIS_SERVER_PORT;
    connectionOptions.password = REDIS_SERVER_PASSWORD;
    connectionOptions.db = REDIS_SERVER_DATABASE;
    ConnectionPoolOptions connectionPoolOptions;
    connectionPoolOptions.size = REDIS_CONNECTION_POOL_SIZE;
    std::shared_ptr<Redis> redisConn = std::make_shared<Redis>(connectionOptions, connectionPoolOptions);
    IMStore::getInstance()->setRedisConn(redisConn);

    UserController userController(sqlConnPool, redisConn);
    FriendController friendController(sqlConnPool, redisConn);
    GroupController groupController(sqlConnPool, redisConn);
    MsgController msgController(sqlConnPool, redisConn);
    FileController fileController(sqlConnPool, redisConn);
    LiveController liveController(sqlConnPool, redisConn);
    AVCallController avCallController(sqlConnPool, redisConn);
    PyqController pyqController(sqlConnPool, redisConn);

    std::thread businessTcpServerThread([&]() {
        std::shared_ptr<AppTcpServer> businessTcpServer = std::make_shared<AppTcpServer>(8888, false);
        businessTcpServer->setConnectionCallback([&](const TcpConnectionPtr& connPtr) {
            if (connPtr->disconnected()) {
                IMStore::getInstance()->removeBusinessTcpConnectionByConn(connPtr);
            }
        });
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_BUSINESS_AUTH_MSG, userController, handleBusinessAuthMsg)
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_SYNC_COMPLETE_MESSAGE, userController, handleSyncCompleteMsg)
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_PERSON_MESSAGE_SEND, msgController, handlePersonSendMsg)
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_GROUP_MESSAGE_SEND, msgController, handleGroupSendMsg)
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_LIVE_ROOM_MSG_SEND, liveController, handleLiveRoomSendMsg)
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_VIDEO_CALL_REQUEST, avCallController, handleVideoCallRequest)
        ADD_BUSINESS_TCP_ENDPOINT(PROTOCOL_TYPE_VIDEO_CALL_RESPONSE, avCallController, handleVideoCallResponse)
        businessTcpServer->start();
    });

    std::thread mediaTcpServerThread([&]() {
        std::shared_ptr<AppTcpServer> mediaTcpServer = std::make_shared<AppTcpServer>(12121, false);
        mediaTcpServer->setMode(MEDIA_MODE);
        mediaTcpServer->setConnectionCallback([&](const TcpConnectionPtr& connPtr) {
            if (connPtr->disconnected()) {
                IMStore::getInstance()->removeMediaTcpConnectionByConn(connPtr);
            }
        });
        mediaTcpServer->setSockOptCallback([](int sockfd) {
            int sendBufSize = 8 * 1024 * 1024;
            setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&sendBufSize, sizeof(sendBufSize));
            int recvBufSize = 8 * 1024 * 1024;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&recvBufSize, sizeof(recvBufSize));
        });
        ADD_MEDIA_TCP_ENDPOINT(PROTOCOL_TYPE_MEDIA_AUTH_MSG, avCallController, handleMediaAuthMsg)
        ADD_MEDIA_TCP_ENDPOINT(PROTOCOL_TYPE_VIDEO_CALL_AUDIO_FRAME, avCallController, handleVideoCallAudioFrame)
        ADD_MEDIA_TCP_ENDPOINT(PROTOCOL_TYPE_VIDEO_CALL_VIDEO_FRAME, avCallController, handleVideoCallVideoFrame)
        mediaTcpServer->start();
    });

    std::thread httpServerThread([&]() {
        std::shared_ptr<HttpServer> httpServer = std::make_shared<HttpServer>(9999);
        ADD_HTTP_MOUNTPOINT("/static/", "/home/linhaojun/cpp-code/cooper-im-server/static")

        // GET
        ADD_HTTP_ENDPOINT("GET", "/live/getOpenedLives", liveController, getOpenedLives)

        // POST
        ADD_HTTP_ENDPOINT("POST", "/user/getVFCode", userController, getVfCode)
        ADD_HTTP_ENDPOINT("POST", "/user/login", userController, userLogin)
        ADD_HTTP_ENDPOINT("POST", "/user/register", userController, userRegister)
        ADD_HTTP_ENDPOINT("POST", "/user/getSyncState", userController, getSyncState)
        ADD_HTTP_ENDPOINT("POST", "/friend/getAllFriends", friendController, getAllFriends)
        ADD_HTTP_ENDPOINT("POST", "/friend/getFriendsByIds", friendController, getFriendsByIds)
        ADD_HTTP_ENDPOINT("POST", "/friend/getSyncFriends", friendController, getSyncFriends)
        ADD_HTTP_ENDPOINT("POST", "/friend/searchFriend", friendController, searchFriend)
        ADD_HTTP_ENDPOINT("POST", "/friend/addFriend", friendController, addFriend)
        ADD_HTTP_ENDPOINT("POST", "/friend/responseFriendApply", friendController, responseFriendApply)
        ADD_HTTP_ENDPOINT("POST", "/group/createGroup", groupController, createGroup)
        ADD_HTTP_ENDPOINT("POST", "/group/searchGroup", groupController, searchGroup)
        ADD_HTTP_ENDPOINT("POST", "/group/addGroup", groupController, addGroup)
        ADD_HTTP_ENDPOINT("POST", "/group/responseGroupApply", groupController, responseGroupApply)
        ADD_HTTP_ENDPOINT("POST", "/group/getAllGroups", groupController, getAllGroups)
        ADD_HTTP_ENDPOINT("POST", "/msg/getAllPersonMessages", msgController, getAllPersonMessages)
        ADD_HTTP_ENDPOINT("POST", "/msg/getSyncPersonMessages", msgController, getSyncPersonMessages)
        ADD_HTTP_ENDPOINT("POST", "/msg/getAllGroupMessages", msgController, getAllGroupMessages)
        ADD_HTTP_ENDPOINT("POST", "/msg/getSyncGroupMessages", msgController, getSyncGroupMessages)
        ADD_HTTP_ENDPOINT("POST", "/file/checkBeforeUpload", fileController, checkBeforeUpload)
        ADD_HTTP_ENDPOINT("POST", "/file/upload", fileController, upload)
        ADD_HTTP_ENDPOINT("POST", "/file/shardUpload", fileController, shardUpload)
        ADD_HTTP_ENDPOINT("POST", "/live/openLive", liveController, openLive)
        ADD_HTTP_ENDPOINT("POST", "/live/closeLive", liveController, closeLive)
        ADD_HTTP_ENDPOINT("POST", "/live/enterLive", liveController, enterLive)
        ADD_HTTP_ENDPOINT("POST", "/live/leaveLive", liveController, leaveLive)
        ADD_HTTP_ENDPOINT("POST", "/live/getOpenedLiveInfoByRoomId", liveController, getOpenedLiveInfoByRoomId)
        ADD_HTTP_ENDPOINT("POST", "/pyq/postPyq", pyqController, postPyq)
        ADD_HTTP_ENDPOINT("POST", "/pyq/getPyq", pyqController, getPyq)
        httpServer->start();
    });

    businessTcpServerThread.join();
    mediaTcpServerThread.join();
    httpServerThread.join();
    return 0;
}
