#include "IMUtil.hpp"

#include <uuid/uuid.h>

#include <store/IMStore.hpp>

std::string IMUtil::generateUUid() {
    uuid_t uuid;
    uuid_generate(uuid);
    char str[37];
    uuid_unparse(uuid, str);
    return {str};
}

long IMUtil::getCurYearRemainingSeconds() {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    time_t nowTime = std::chrono::system_clock::to_time_t(now);
    struct tm* current_time = std::localtime(&nowTime);

    // 计算剩余时间
    struct tm endOfYear = {0};
    endOfYear.tm_year = current_time->tm_year;  // 当前年份
    endOfYear.tm_mon = 11;                      // 月份为11表示12月（月份从0开始）
    endOfYear.tm_mday = 31;                     // 12月31日
    endOfYear.tm_hour = 23;                     // 23时
    endOfYear.tm_min = 59;                      // 59分
    endOfYear.tm_sec = 59;                      // 59秒

    // 将结束时间转换为时间戳
    std::time_t endOfYearTime = std::mktime(&endOfYear);

    // 计算剩余秒数
    std::chrono::seconds remainingSeconds = std::chrono::seconds(endOfYearTime - nowTime);

    return remainingSeconds.count();
}

int IMUtil::getCurrentYear() {
    auto now = std::chrono::system_clock::now();
    time_t now_c = std::chrono::system_clock::to_time_t(now);
    struct tm* current_time = std::localtime(&now_c);
    return current_time->tm_year + 1900;
}

std::string IMUtil::generateGroupNum() {
    // 群号一共是十位
    // 前两位位是根据年份决定的，比如：23 (23年)
    // 后八位当天从00000001递增
    // 23 00000001
    std::string year = std::to_string(getCurrentYear()).substr(2, 2);
    auto redisConn = IMStore::getInstance()->getRedisConn();
    if (!redisConn->exists(REDIS_KEY_CUR_YEAR_GROUP_NUM)) {
        redisConn->incr(REDIS_KEY_CUR_YEAR_GROUP_NUM);
        // 设置过期时间为当年剩下的时间
        redisConn->expire(REDIS_KEY_CUR_YEAR_GROUP_NUM, IMUtil::getCurYearRemainingSeconds());
        return year + "00000001";
    } else {
        auto num = redisConn->incr(REDIS_KEY_CUR_YEAR_GROUP_NUM);
        std::string numStr = std::to_string(num);
        while (numStr.size() < 8) {
            numStr = std::to_string(0).append(numStr);
        }
        return year + numStr;
    }
}
