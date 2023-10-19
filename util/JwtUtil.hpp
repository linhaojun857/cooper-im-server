#ifndef util_JwtUtil_hpp
#define util_JwtUtil_hpp

#include "jwt-cpp/jwt.h"

class JwtUtil {
public:
    static int parseToken(const std::string& token);

    static std::string createToken(int id);
};

#endif
