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

    void getAllFriends(const HttpRequest& request, HttpResponse& response);

    void getFriendsByIds(const HttpRequest& request, HttpResponse& response);

    void getSyncFriends(const HttpRequest& request, HttpResponse& response);

    void searchFriend(const HttpRequest& request, HttpResponse& response);

    void addFriend(const HttpRequest& request, HttpResponse& response);

    void responseFriendApply(const HttpRequest& request, HttpResponse& response);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
