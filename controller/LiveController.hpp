#ifndef controller_liveController_hpp
#define controller_liveController_hpp

#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class LiveController {
public:
    LiveController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void openLive(const HttpRequest& request, HttpResponse& response);

    void closeLive(const HttpRequest& request, HttpResponse& response);

    void getOpenedLives(const HttpRequest& request, HttpResponse& response);

    void getOpenedLiveInfoByRoomId(const HttpRequest& request, HttpResponse& response);

    void enterLive(const HttpRequest& request, HttpResponse& response);

    void leaveLive(const HttpRequest& request, HttpResponse& response);

    void handleLiveRoomMsg(const TcpConnectionPtr& connPtr, const json& params);

    void notifyUsersWhenLiveEnd(int roomId);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
