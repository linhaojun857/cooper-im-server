#include <sw/redis++/redis++.h>

#include <iostream>

#include "define/IMDefine.hpp"
#include "util/IMUtil.hpp"
#include "store/IMStore.hpp"

using namespace sw::redis;

int main() {
    ConnectionOptions connectionOptions;
    connectionOptions.host = REDIS_SERVER_IP;
    connectionOptions.port = REDIS_SERVER_PORT;
    connectionOptions.password = REDIS_SERVER_PASSWORD;
    connectionOptions.db = REDIS_SERVER_DATABASE;
    std::shared_ptr<Redis> redisConn = std::make_shared<Redis>(connectionOptions);
    IMStore::getInstance()->setRedisConn(redisConn);
    std::cout << IMUtil::generateGroupNum() << std::endl;
    return 0;
}
