#ifndef Entity_hpp_
#define Entity_hpp_

#include <mysql.hpp>
#include <string>

struct User {
    int id;
    std::string username;
    std::string nickname;
    std::string password;
};

REFLECTION(User, id, username, nickname, password)

#endif
