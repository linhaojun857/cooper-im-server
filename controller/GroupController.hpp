#ifndef controller_GroupController_hpp
#define controller_GroupController_hpp

#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class GroupController {
public:
    GroupController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void createGroup(const HttpRequest& request, HttpResponse& response);

    void searchGroup(const HttpRequest& request, HttpResponse& response);

    void addGroup(const HttpRequest& request, HttpResponse& response);

    void responseGroupApply(const HttpRequest& request, HttpResponse& response);

    void getAllGroups(const HttpRequest& request, HttpResponse& response);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
