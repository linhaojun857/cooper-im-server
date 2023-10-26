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

    User(int id, const std::string& username, const std::string& nickname, const std::string& avatar,
         const std::string& status, const std::string& feeling) {
        this->id = id;
        this->username = username;
        this->nickname = nickname;
        this->avatar = avatar;
        this->status = status;
        this->feeling = feeling;
    }

    static std::shared_ptr<User> fromJson(const json& j) {
        if (j.contains("username")) {
            std::cout << j["username"].get<std::string>() << std::endl;
        }
        auto user = std::make_shared<User>(j["id"].get<int>(), j["username"].get<std::string>(),
                                           j["nickname"].get<std::string>(), j["avatar"].get<std::string>(),
                                           j["status"].get<std::string>(), j["feeling"].get<std::string>());
        return user;
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

    [[nodiscard]] json toJson() const {
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

    Notify() = default;

    Notify(int id, int to_id, int type, int fa_id) {
        this->id = id;
        this->to_id = to_id;
        this->notify_type = type;
        this->fa_id = fa_id;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["to_id"] = to_id;
        j["notify_type"] = notify_type;
        j["fa_id"] = fa_id;
        return j;
    }
};
REFLECTION(Notify, id, to_id, notify_type, fa_id)

struct PersonMessage {
    int id{};
    int from_id{};
    int to_id{};
    int message_type{};
    std::string message;
    std::string file_url;
    time_t timestamp{};

    PersonMessage() = default;

    PersonMessage(int from_id, int to_id, int message_type, const std::string& message, const std::string& file_url,
                  time_t timestamp) {
        this->id = 0;
        this->from_id = from_id;
        this->to_id = to_id;
        this->message_type = message_type;
        this->message = message;
        this->file_url = file_url;
        this->timestamp = timestamp;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["from_id"] = from_id;
        j["to_id"] = to_id;
        j["message_type"] = message_type;
        j["message"] = message;
        j["file_url"] = file_url;
        j["timestamp"] = timestamp;
        return j;
    }
};

REFLECTION(PersonMessage, id, from_id, to_id, message_type, message, file_url, timestamp)

struct SyncState {
    int id{};
    int user_id{};
    int friend_sync_state{};
    std::string updated_friends;

    SyncState() = default;

    explicit SyncState(int userId) {
        this->user_id = userId;
        updated_friends = "[]";
    }

    void addUpdatedFriend(int friendId) {
        json j = json::parse(updated_friends);
        j.push_back(friendId);
        updated_friends = j.dump();
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["user_id"] = user_id;
        j["friend_sync_state"] = friend_sync_state;
        j["updated_friends"] = updated_friends;
        return j;
    }
};

REFLECTION(SyncState, id, user_id, friend_sync_state, updated_friends)

#endif
