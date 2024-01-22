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

    void createGroup(HttpRequest& request, HttpResponse& response);

    void searchGroup(HttpRequest& request, HttpResponse& response);

    void addGroup(HttpRequest& request, HttpResponse& response);

    void responseGroupApply(HttpRequest& request, HttpResponse& response);

    void getAllGroups(HttpRequest& request, HttpResponse& response);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
