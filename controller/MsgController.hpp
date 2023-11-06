#ifndef controller_MsgController_hpp
#define controller_MsgController_hpp

#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class MsgController {
public:
    MsgController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params);

    void getAllPersonMessages(const HttpRequest& request, HttpResponse& response);

    void getSyncPersonMessages(const HttpRequest& request, HttpResponse& response);

    void handleGroupSendMsg(const TcpConnectionPtr& connPtr, const json& params);

    void getAllGroupMessages(const HttpRequest& request, HttpResponse& response);

    void getSyncGroupMessages(const HttpRequest& request, HttpResponse& response);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
