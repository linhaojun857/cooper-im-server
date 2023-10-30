#ifndef controller_MsgController_hpp
#define controller_MsgController_hpp

#include <sw/redis++/redis++.h>

#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class MsgController {
public:
    explicit MsgController(std::shared_ptr<dbng<mysql>> mysql, std::shared_ptr<Redis> redisConn);

    void handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params);

    void getAllPersonMessages(const HttpRequest& request, HttpResponse& response);

    void getSyncPersonMessages(const HttpRequest& request, HttpResponse& response);

private:
    std::shared_ptr<dbng<mysql>> sqlConn_;
    std::shared_ptr<Redis> redisConn_;
};

#endif
