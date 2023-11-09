#ifndef util_IMUtil_hpp
#define util_IMUtil_hpp

#include <string>

class IMUtil {
public:
    static std::string generateUUid();

    static long getCurYearRemainingSeconds();

    static int getCurrentYear();

    static std::string generateGroupNum();

    static std::string generateRandomFileName(const std::string& fileType);

    static std::string getFileType(const std::string& filename);
};

#endif
