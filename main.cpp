#include <cooper/net/AppTcpServer.hpp>
#include <cooper/net/HttpServer.hpp>
#include <cooper/util/AsyncLogWriter.hpp>
#include <dbng.hpp>
#include <mysql.hpp>
#include <regex>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "service/UserService.hpp"

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
    Logger::setOutputFunction(std::bind(&AsyncLogWriter::write, &writer, std::placeholders::_1, std::placeholders::_2),
                              std::bind(&AsyncLogWriter::flushAll, &writer));
    std::shared_ptr<dbng<mysql>> sqlConn = std::make_shared<dbng<mysql>>();
    if (!sqlConn->connect(MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME, MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE)) {
        LOG_ERROR << "connect mysql failed";
        return -1;
    }
    if (!sqlConn->create_datatable<User>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Friend>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table user failed";
        return -1;
    }
    UserService userService(sqlConn);
    std::thread appTcpServerThread([&]() {
        AppTcpServer appTcpServer(8888, false);
        appTcpServer.start();
    });
    std::thread httpServerThread([&]() {
        HttpServer httpServer(9999);
        ADD_MOUNTPOINT("/static/", "/home/linhaojun/cpp-code/cooper-im-server/static");
        ADD_ENDPOINT("POST", "/user/login", userService, &UserService::userLogin);
        ADD_ENDPOINT("POST", "/user/register", userService, &UserService::userRegister);
        ADD_ENDPOINT("POST", "/user/getVFCode", userService, &UserService::getVfCode);
        ADD_ENDPOINT("POST", "/user/search", userService, &UserService::search);
        httpServer.start();
    });
    appTcpServerThread.join();
    httpServerThread.join();
    return 0;
}
