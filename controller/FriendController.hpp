#ifndef controller_FriendController_hpp
#define controller_FriendController_hpp

#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class FriendController {
public:
    FriendController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void getAllFriends(HttpRequest& request, HttpResponse& response);

    void getFriendsByIds(HttpRequest& request, HttpResponse& response);

    void getSyncFriends(HttpRequest& request, HttpResponse& response);

    void searchFriend(HttpRequest& request, HttpResponse& response);

    void addFriend(HttpRequest& request, HttpResponse& response);

    void responseFriendApply(HttpRequest& request, HttpResponse& response);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
