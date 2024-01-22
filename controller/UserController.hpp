#ifndef controller_UserController_hpp
#define controller_UserController_hpp
#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class UserController {
public:
    UserController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void getVfCode(HttpRequest& request, HttpResponse& response);

    void userLogin(HttpRequest& request, HttpResponse& response);

    void userRegister(HttpRequest& request, HttpResponse& response);

    void getSyncState(HttpRequest& request, HttpResponse& response);

    void handleAuthMsg(const TcpConnectionPtr& connPtr, const json& params);

    void handleSyncCompleteMsg(const TcpConnectionPtr& connPtr, const json& params);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
