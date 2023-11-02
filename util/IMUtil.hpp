#ifndef util_IMUtil_hpp
#define util_IMUtil_hpp

#include <string>

class IMUtil {
public:
    static std::string generateUUid();

    static long getCurYearRemainingSeconds();

    static int getCurrentYear();

    static std::string generateGroupNum();
};

#endif
