#ifndef Entity_hpp_
#define Entity_hpp_

#include <mysql.hpp>
#include <string>

struct User {
    int id;
    std::string username;
    std::string nickname;
    std::string password;
    std::string avatar;
    std::string status;
    std::string feeling;
};
REFLECTION(User, id, username, nickname, password, avatar, status, feeling)

struct Friend {
    int id;
    int a_id;
    int b_id;
    // 0: 我的好友 1: 特别关心 2: 家人 ...
    int group_type;
};
REFLECTION(Friend, id, a_id, b_id, group_type)

#endif
