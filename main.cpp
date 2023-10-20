#include <cooper/net/AppTcpServer.hpp>
#include <cooper/net/HttpServer.hpp>
#include <cooper/util/AsyncLogWriter.hpp>
#include <dbng.hpp>
#include <mysql.hpp>
#include <regex>

#include "controller//UserController.hpp"
#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"

#define ADD_MOUNTPOINT(mountPoint, dir) \
    Headers headers;                    \
    httpServer.addMountPoint(mountPoint, dir, headers)

#define ADD_ENDPOINT(method, path, service, handler) \
    httpServer.addEndpoint(method, path, std::bind(handler, &service, _1, _2))

using namespace cooper;
using namespace ormpp;
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
        !sqlConn->create_datatable<FriendApply>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table user failed";
        return -1;
    }
    UserController userController(sqlConn);
    std::thread appTcpServerThread([&]() {
        AppTcpServer appTcpServer(8888, false);
        appTcpServer.start();
    });
    std::thread httpServerThread([&]() {
        HttpServer httpServer(9999);
        ADD_MOUNTPOINT("/static/", "/home/linhaojun/cpp-code/cooper-im-server/static");
        ADD_ENDPOINT("POST", "/user/login", userController, &UserController::userLogin);
        ADD_ENDPOINT("POST", "/user/register", userController, &UserController::userRegister);
        ADD_ENDPOINT("POST", "/user/getVFCode", userController, &UserController::getVfCode);
        ADD_ENDPOINT("POST", "/user/search", userController, &UserController::search);
        ADD_ENDPOINT("POST", "/user/addFriend", userController, &UserController::addFriend);
        httpServer.start();
    });
    appTcpServerThread.join();
    httpServerThread.join();
    return 0;
}
