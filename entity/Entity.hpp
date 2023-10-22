#ifndef Entity_hpp_
#define Entity_hpp_

#include <mysql.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace ormpp;
using namespace nlohmann;

struct User {
    int id{};
    std::string username;
    std::string nickname;
    std::string password;
    std::string avatar;
    std::string status;
    std::string feeling;

    User() = default;

    User(int id, const std::string& username, const std::string& nickname, const std::string& password,
         const std::string& avatar, const std::string& status, const std::string& feeling) {
        this->id = id;
        this->username = username;
        this->nickname = nickname;
        this->password = password;
        this->avatar = avatar;
        this->status = status;
        this->feeling = feeling;
    }

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

    Friend() = default;

    Friend(int id, int a_id, int b_id, int group_type) {
        this->id = id;
        this->a_id = a_id;
        this->b_id = b_id;
        this->group_type = group_type;
    }
};
REFLECTION(Friend, id, a_id, b_id, group_type)

struct FriendApply {
    int id{};
    int from_id{};
    int to_id{};
    std::string from_avatar;
    std::string from_nickname;
    std::string to_avatar;
    std::string to_nickname;
    std::string reason;
    // 0: 待处理 1: 通过申请 2: 拒绝申请
    int agree{};

    FriendApply() = default;

    FriendApply(int id, int from_id, int to_id, const std::string& from_avatar, const std::string& from_nickname,
                const std::string& to_avatar, const std::string& to_nickname, const std::string& reason, int agree) {
        this->id = id;
        this->from_id = from_id;
        this->to_id = to_id;
        this->from_avatar = from_avatar;
        this->from_nickname = from_nickname;
        this->to_avatar = to_avatar;
        this->to_nickname = to_nickname;
        this->reason = reason;
        this->agree = agree;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["from_id"] = from_id;
        j["to_id"] = to_id;
        j["from_avatar"] = from_avatar;
        j["from_nickname"] = from_nickname;
        j["to_avatar"] = to_avatar;
        j["to_nickname"] = to_nickname;
        j["reason"] = reason;
        j["agree"] = agree;
        return j;
    }
};
REFLECTION(FriendApply, id, from_id, to_id, from_avatar, from_nickname, to_avatar, to_nickname, agree)

struct Notify {
    int id;
    // 推送给谁
    int to_id;
    // 0: 好友申请...
    int notify_type;
    int fa_id;
    int is_complete;

    Notify() = default;

    Notify(int id, int to_id, int type, int fa_id, int is_complete) {
        this->id = id;
        this->to_id = to_id;
        this->notify_type = type;
        this->fa_id = fa_id;
        this->is_complete = is_complete;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["to_id"] = to_id;
        j["notify_type"] = notify_type;
        j["fa_id"] = fa_id;
        j["is_complete"] = is_complete;
        return j;
    }
};
REFLECTION(Notify, id, to_id, notify_type, fa_id, is_complete)

#endif
