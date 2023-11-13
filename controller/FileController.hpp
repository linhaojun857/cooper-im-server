#ifndef controller_FileController_hpp
#define controller_FileController_hpp

#include <sw/redis++/redis++.h>

#include <connection_pool.hpp>
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

class FileController {
public:
    FileController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn);

    void checkBeforeUpload(const HttpRequest& request, HttpResponse& response);

    void upload(const HttpRequest& request, HttpResponse& response);

    void shardUpload(const HttpRequest& request, HttpResponse& response);

private:
    connection_pool<dbng<mysql>>* sqlConnPool_ = nullptr;
    std::shared_ptr<Redis> redisConn_;
};

#endif
