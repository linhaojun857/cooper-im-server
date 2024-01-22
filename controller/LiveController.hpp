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

    void openLive(HttpRequest& request, HttpResponse& response);

    void closeLive(HttpRequest& request, HttpResponse& response);

    void getOpenedLives(HttpRequest& request, HttpResponse& response);

    void getOpenedLiveInfoByRoomId(HttpRequest& request, HttpResponse& response);

    void enterLive(HttpRequest& request, HttpResponse& response);

    void leaveLive(HttpRequest& request, HttpResponse& response);

    void handleLiveRoomSendMsg(const TcpConnectionPtr& connPtr, const json& params);

    void notifyUsersWhenLiveEnd(int roomId);

    void updateLiveRoomViewerCount(int roomId, int count, int enterViewerId);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
