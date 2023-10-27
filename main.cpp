#include <sw/redis++/redis++.h>

#include <cooper/net/AppTcpServer.hpp>
#include <cooper/net/HttpServer.hpp>
#include <cooper/util/AsyncLogWriter.hpp>
#include <dbng.hpp>
#include <mysql.hpp>
#include <regex>

#include "controller//UserController.hpp"
#include "controller/MsgController.hpp"
#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"

#define ADD_HTTP_MOUNTPOINT(mountPoint, dir) \
    Headers headers;                         \
    httpServer.addMountPoint(mountPoint, dir, headers);

#define ADD_HTTP_ENDPOINT(method, path, service, handler) \
    httpServer.addEndpoint(method, path, std::bind(handler, &service, _1, _2));

#define ADD_TCP_ENDPOINT(type, service, handler) \
    appTcpServer.registerProtocolHandler(type, std::bind(handler, &service, _1, _2));

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;
using namespace std::placeholders;

int main() {
    AsyncLogWriter writer;
    Logger::setLogLevel(Logger::kTrace);
    Logger::setOutputFunction(std::bind(&AsyncLogWriter::write, &writer, _1, _2),
                              std::bind(&AsyncLogWriter::flushAll, &writer));
    std::shared_ptr<dbng<mysql>> sqlConn = std::make_shared<dbng<mysql>>();
    if (!sqlConn->connect(MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME, MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE)) {
        LOG_ERROR << "connect mysql failed";
        return -1;
    }
    if (!sqlConn->create_datatable<User>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Friend>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Notify>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<FriendApply>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<PersonMessage>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table  failed";
        return -1;
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
    UserController userController(sqlConn);
    MsgController msgController(sqlConn);
    std::thread appTcpServerThread([&]() {
        AppTcpServer appTcpServer(8888, false);
        appTcpServer.setConnectionCallback([&](const TcpConnectionPtr& connPtr) {
            if (connPtr->disconnected()) {
                IMStore::getInstance()->removeTcpConnectionByConn(connPtr);
            }
        });
        ADD_TCP_ENDPOINT(PROTOCOL_TYPE_AUTH_MSG, userController, &UserController::handleAuthMsg)
        ADD_TCP_ENDPOINT(PROTOCOL_TYPE_SYNC_COMPLETE_MESSAGE, userController, &UserController::handleSyncCompleteMsg)
        ADD_TCP_ENDPOINT(PROTOCOL_TYPE_PERSON_MESSAGE, msgController, &MsgController::handlePersonSendMsg)
        appTcpServer.start();
    });
    std::thread httpServerThread([&]() {
        HttpServer httpServer(9999);
        ADD_HTTP_MOUNTPOINT("/static/", "/home/linhaojun/cpp-code/cooper-im-server/static")
        ADD_HTTP_ENDPOINT("POST", "/user/login", userController, &UserController::userLogin)
        ADD_HTTP_ENDPOINT("POST", "/user/register", userController, &UserController::userRegister)
        ADD_HTTP_ENDPOINT("POST", "/user/getAllFriends", userController, &UserController::getAllFriends)
        ADD_HTTP_ENDPOINT("POST", "/user/getFriendsByIds", userController, &UserController::getFriendsByIds)
        ADD_HTTP_ENDPOINT("POST", "/user/getSyncState", userController, &UserController::getSyncState)
        ADD_HTTP_ENDPOINT("POST", "/user/getVFCode", userController, &UserController::getVfCode)
        ADD_HTTP_ENDPOINT("POST", "/user/search", userController, &UserController::search)
        ADD_HTTP_ENDPOINT("POST", "/user/addFriend", userController, &UserController::addFriend)
        ADD_HTTP_ENDPOINT("POST", "/user/responseFriendApply", userController, &UserController::responseFriendApply)
        httpServer.start();
    });
    appTcpServerThread.join();
    httpServerThread.join();
    return 0;
}
