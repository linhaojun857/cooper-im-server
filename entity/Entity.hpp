#ifndef Entity_hpp_
#define Entity_hpp_

#include <mysql.hpp>
#include <nlohmann/json.hpp>
#include <string>

#include "define/IMDefine.hpp"

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
REFLECTION_WITH_NAME(User, "t_user", id, username, nickname, password, avatar, status, feeling)

struct UserDTO {
    int id{};
    std::string username;
    std::string nickname;
    std::string password;
    std::string avatar;
    std::string status;
    std::string feeling;
    std::string session_id;

    json toJson() {
        json j;
        j["id"] = id;
        j["username"] = username;
        j["nickname"] = nickname;
        j["avatar"] = avatar;
        j["status"] = status;
        j["feeling"] = feeling;
        j["session_id"] = session_id;
        return j;
    }
};

REFLECTION(UserDTO, id, username, nickname, password, avatar, status, feeling, session_id)

struct Friend {
    int id{};
    int a_id{};
    int b_id{};
    std::string session_id;
    // 0: 我的好友 1: 特别关心 2: 家人 ...
    int group_type{};

    Friend() = default;

    Friend(int id, int a_id, int b_id, const std::string& session_id, int group_type) {
        this->id = id;
        this->a_id = a_id;
        this->b_id = b_id;
        this->session_id = session_id;
        this->group_type = group_type;
    }
};
REFLECTION_WITH_NAME(Friend, "t_friend", id, a_id, b_id, session_id, group_type)

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

REFLECTION_WITH_NAME(FriendApply, "t_friend_apply", id, from_id, to_id, from_avatar, from_nickname, to_avatar,
                     to_nickname, agree)

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
REFLECTION_WITH_NAME(Notify, "t_notify", id, to_id, notify_type, fa_id)

struct PersonMessage {
    int id{};
    int from_id{};
    int to_id{};
    std::string session_id;
    int message_type{};
    std::string message;
    std::string file_url;
    time_t timestamp{};

    PersonMessage() = default;

    PersonMessage(int from_id, int to_id, const std::string& session_id, int message_type, const std::string& message,
                  const std::string& file_url, time_t timestamp) {
        this->id = 0;
        this->from_id = from_id;
        this->to_id = to_id;
        this->session_id = session_id;
        this->message_type = message_type;
        this->message = message;
        this->file_url = file_url;
        this->timestamp = timestamp;
    }

    static PersonMessage fromJson(const json& j) {
        PersonMessage pm(j["from_id"].get<int>(), j["to_id"].get<int>(), j["session_id"].get<std::string>(),
                         j["message_type"].get<int>(), j["message"].get<std::string>(),
                         j["file_url"].get<std::string>(), j["timestamp"].get<time_t>());
        return pm;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["from_id"] = from_id;
        j["to_id"] = to_id;
        j["session_id"] = session_id;
        j["message_type"] = message_type;
        j["message"] = message;
        j["file_url"] = file_url;
        j["timestamp"] = timestamp;
        return j;
    }
};

REFLECTION_WITH_NAME(PersonMessage, "t_person_message", id, from_id, to_id, session_id, message_type, message, file_url,
                     timestamp)

struct Group {
    int id{};
    std::string session_id;
    std::string group_num;
    int owner_id{};
    std::string name;
    std::string avatar;
    std::string description;

    Group() = default;

    Group(const std::string& session_id, const std::string& group_num, int owner_id, const std::string& name,
          const std::string& avatar, const std::string& description) {
        this->session_id = session_id;
        this->group_num = group_num;
        this->owner_id = owner_id;
        this->name = name;
        this->avatar = avatar;
        this->description = description;
    }

    static Group fromJson(json j) {
        Group group(j["session_id"].get<std::string>(), j["group_num"].get<std::string>(), j["owner_id"].get<int>(),
                    j["name"].get<std::string>(), j["avatar"].get<std::string>(), j["desc"].get<std::string>());
        return group;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["session_id"] = session_id;
        j["group_num"] = group_num;
        j["owner_id"] = owner_id;
        j["name"] = name;
        j["avatar"] = avatar;
        j["description"] = description;
        return j;
    }
};

REFLECTION_WITH_NAME(Group, "t_group", id, session_id, group_num, owner_id, name, avatar, description)

struct UserGroup {
    int id{};
    int user_id{};
    int group_id{};
};

REFLECTION_WITH_NAME(UserGroup, "t_user_group", id, user_id, group_id)

struct SyncState {
    int user_id{};
    int friend_sync_state{};
    int person_message_sync_state{};
    std::vector<std::pair<int, int>> friendIds;
    std::vector<std::pair<int, int>> personMessageIds;

    SyncState() = default;

    explicit SyncState(int userId) {
        this->user_id = userId;
        friend_sync_state = 0;
        person_message_sync_state = 0;
    }

    void addInsertedFriendId(int friendId) {
        friendIds.emplace_back(friendId, SYNC_DATA_FRIEND_ENTITY_INSERT);
    }

    void addUpdatedFriendId(int friendId) {
        friendIds.emplace_back(friendId, SYNC_DATA_FRIEND_ENTITY_UPDATE);
    }

    void addDeletedFriendId(int friendId) {
        friendIds.emplace_back(friendId, SYNC_DATA_FRIEND_ENTITY_DELETE);
    }

    void addInsertedPersonMessageId(int personMessageId) {
        personMessageIds.emplace_back(personMessageId, SYNC_DATA_PERSON_MESSAGE_INSERT);
    }

    void addDeletedPersonMessageId(int personMessageId) {
        personMessageIds.emplace_back(personMessageId, SYNC_DATA_PERSON_MESSAGE_DELETE);
    }

    void clearAllFriendIds() {
        friendIds.clear();
    }

    void clearAllPersonMessageIds() {
        personMessageIds.clear();
    }

    static SyncState fromJson(const json& j) {
        SyncState state(j["user_id"].get<int>());
        state.friend_sync_state = j["friend_sync_state"].get<int>();
        state.person_message_sync_state = j["person_message_sync_state"].get<int>();
        state.friendIds = j["friendIds"].get<std::vector<std::pair<int, int>>>();
        state.personMessageIds = j["personMessageIds"].get<std::vector<std::pair<int, int>>>();
        return state;
    }

    json toJson() {
        json j;
        j["user_id"] = user_id;
        j["friend_sync_state"] = friend_sync_state;
        j["person_message_sync_state"] = person_message_sync_state;
        j["friendIds"] = friendIds;
        j["personMessageIds"] = personMessageIds;
        return j;
    }
};

#endif
