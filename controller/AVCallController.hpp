#ifndef controller_VideoCallController_hpp
#define controller_VideoCallController_hpp

#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class AVCallController {
public:
    AVCallController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void handleVideoCallRequest(const TcpConnectionPtr& connPtr, const json& params);

    static void handleVideoCallResponse(const TcpConnectionPtr& connPtr, const json& params);

    static void handleMediaAuthMsg(const TcpConnectionPtr& connPtr, const char* buf, size_t len);

    static void handleVideoCallAudioFrame(const TcpConnectionPtr& connPtr, const char* buf, size_t len);

    static void handleVideoCallVideoFrame(const TcpConnectionPtr& connPtr, const char* buf, size_t len);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
