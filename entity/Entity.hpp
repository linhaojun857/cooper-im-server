#ifndef Entity_hpp_
#define Entity_hpp_

#include <mysql.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace ormpp;
using namespace nlohmann;

struct User {
    int id;
    std::string username;
    std::string nickname;
    std::string password;
    std::string avatar;
    std::string status;
    std::string feeling;

    json toJson() {
        json j;
        j["id"] = id;
        j["username"] = username;
        j["nickname"] = nickname;
        j["avatar"] = avatar;
        j["status"] = status;
        j["feeling"] = feeling;
        return j;
    }
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

struct FriendApply {
    int id;
    int from_id;
    int to_id;
    std::string avatar;
    std::string nickname;
    std::string reason;
    // 0: 待处理 1: 通过申请 2: 拒绝申请
    int agree;
};
REFLECTION(FriendApply, id, from_id, to_id, avatar, nickname, agree)

struct Notify {
    int id;
    // 推送给谁
    int to_id;
    // 0: 好友申请...
    int type;
    int fa_id;
    int is_complete;
};
REFLECTION(Notify, id, to_id, type, fa_id, is_complete)

#endif
