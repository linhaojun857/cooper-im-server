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

struct GroupApply {
    int id{};
    int from_id{};
    int to_id{};
    int group_id{};
    std::string from_avatar;
    std::string from_nickname;
    std::string to_avatar;
    std::string to_name;
    std::string reason;
    // 0: 待处理 1: 通过申请 2: 拒绝申请
    int agree{};

    GroupApply() = default;

    GroupApply(int id, int from_id, int to_id, int group_id, const std::string& from_avatar,
               const std::string& from_nickname, const std::string& to_avatar, const std::string& to_name,
               const std::string& reason, int agree) {
        this->id = id;
        this->from_id = from_id;
        this->to_id = to_id;
        this->group_id = group_id;
        this->from_avatar = from_avatar;
        this->from_nickname = from_nickname;
        this->to_avatar = to_avatar;
        this->to_name = to_name;
        this->reason = reason;
        this->agree = agree;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["from_id"] = from_id;
        j["to_id"] = to_id;
        j["group_id"] = group_id;
        j["from_avatar"] = from_avatar;
        j["from_nickname"] = from_nickname;
        j["to_avatar"] = to_avatar;
        j["to_name"] = to_name;
        j["reason"] = reason;
        j["agree"] = agree;
        return j;
    }
};

REFLECTION_WITH_NAME(GroupApply, "t_group_apply", id, from_id, to_id, group_id, from_avatar, from_nickname, to_avatar,
                     to_name, agree)

struct Notify {
    int id;
    int to_id;
    // 0: 好友申请 1: 群组申请
    int notify_type;
    int apply_id;

    Notify() = default;

    Notify(int id, int to_id, int type, int apply_id) {
        this->id = id;
        this->to_id = to_id;
        this->notify_type = type;
        this->apply_id = apply_id;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["to_id"] = to_id;
        j["notify_type"] = notify_type;
        j["apply_id"] = apply_id;
        return j;
    }
};
REFLECTION_WITH_NAME(Notify, "t_notify", id, to_id, notify_type, apply_id)

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

struct GroupMessage {
    int id{};
    int from_id{};
    int group_id{};
    int message_type{};
    // 当消息类型为file时，message存储json格式文件信息（文件名，文件大小）
    std::string message;
    std::string file_url;
    time_t timestamp{};

    GroupMessage() = default;

    GroupMessage(int from_id, int group_id, int message_type, const std::string& message, const std::string& file_url,
                 time_t timestamp) {
        this->id = 0;
        this->from_id = from_id;
        this->group_id = group_id;
        this->message_type = message_type;
        this->message = message;
        this->file_url = file_url;
        this->timestamp = timestamp;
    }

    static GroupMessage fromJson(const json& j) {
        GroupMessage gm(j["from_id"].get<int>(), j["group_id"].get<int>(), j["message_type"].get<int>(),
                        j["message"].get<std::string>(), j["file_url"].get<std::string>(),
                        j["timestamp"].get<time_t>());
        return gm;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["from_id"] = from_id;
        j["group_id"] = group_id;
        j["message_type"] = message_type;
        j["message"] = message;
        j["file_url"] = file_url;
        j["timestamp"] = timestamp;
        return j;
    }
};

REFLECTION_WITH_NAME(GroupMessage, "t_group_message", id, from_id, group_id, message_type, message, file_url, timestamp)

struct GroupMessageDTO {
    int id{};
    int from_id{};
    std::string from_nickname;
    std::string from_avatar;
    int group_id{};
    int message_type{};
    std::string message;
    std::string file_url;
    time_t timestamp{};

    GroupMessageDTO() = default;

    explicit GroupMessageDTO(const GroupMessage& gm) {
        this->id = gm.id;
        this->from_id = gm.from_id;
        this->group_id = gm.group_id;
        this->message_type = gm.message_type;
        this->message = gm.message;
        this->file_url = gm.file_url;
        this->timestamp = gm.timestamp;
    }

    GroupMessageDTO(int id, int from_id, const std::string& from_nickname, const std::string& from_avatar, int group_id,
                    int message_type, const std::string& message, const std::string& file_url, time_t timestamp) {
        this->id = id;
        this->from_id = from_id;
        this->from_nickname = from_nickname;
        this->from_avatar = from_avatar;
        this->group_id = group_id;
        this->message_type = message_type;
        this->message = message;
        this->file_url = file_url;
        this->timestamp = timestamp;
    }

    static GroupMessageDTO fromJson(const json& j) {
        GroupMessageDTO gm(j["id"].get<int>(), j["from_id"].get<int>(), j["from_nickname"].get<std::string>(),
                           j["from_avatar"].get<std::string>(), j["group_id"].get<int>(), j["message_type"].get<int>(),
                           j["message"].get<std::string>(), j["file_url"].get<std::string>(),
                           j["timestamp"].get<time_t>());
        return gm;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["from_id"] = from_id;
        j["from_nickname"] = from_nickname;
        j["from_avatar"] = from_avatar;
        j["group_id"] = group_id;
        j["message_type"] = message_type;
        j["message"] = message;
        j["file_url"] = file_url;
        j["timestamp"] = timestamp;
        return j;
    }
};

REFLECTION(GroupMessageDTO, id, from_id, group_id, message_type, message, file_url, timestamp, from_nickname,
           from_avatar)

struct Group {
    int id{};
    std::string session_id;
    std::string group_num;
    int owner_id{};
    std::string name;
    std::string avatar;
    std::string description;
    int member_count{};

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

    Group(const std::string& session_id, const std::string& group_num, int owner_id, const std::string& name,
          const std::string& avatar, const std::string& description, int member_count) {
        this->session_id = session_id;
        this->group_num = group_num;
        this->owner_id = owner_id;
        this->name = name;
        this->avatar = avatar;
        this->description = description;
        this->member_count = member_count;
    }

    static Group fromJson(json j) {
        Group group(j["session_id"].get<std::string>(), j["group_num"].get<std::string>(), j["owner_id"].get<int>(),
                    j["name"].get<std::string>(), j["avatar"].get<std::string>(), j["desc"].get<std::string>(),
                    j["member_count"].get<int>());
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
        j["member_count"] = member_count;
        return j;
    }
};

REFLECTION_WITH_NAME(Group, "t_group", id, session_id, group_num, owner_id, name, avatar, description, member_count)

struct UserGroup {
    int id{};
    int user_id{};
    int group_id{};

    UserGroup() = default;

    UserGroup(int user_id, int group_id) {
        this->user_id = user_id;
        this->group_id = group_id;
    }
};

REFLECTION_WITH_NAME(UserGroup, "t_user_group", id, user_id, group_id)

struct SyncState {
    int user_id{};
    int friend_sync_state{};
    int person_message_sync_state{};
    int group_message_sync_state{};
    std::vector<std::pair<int, int>> friendIds;
    std::vector<std::pair<int, int>> personMessageIds;
    std::vector<std::pair<int, int>> groupMessageIds;

    SyncState() = default;

    explicit SyncState(int userId) {
        this->user_id = userId;
        friend_sync_state = 0;
        person_message_sync_state = 0;
        group_message_sync_state = 0;
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

    void addInsertedGroupMessageId(int groupMessageId) {
        groupMessageIds.emplace_back(groupMessageId, SYNC_DATA_PERSON_MESSAGE_INSERT);
    }

    void addDeletedGroupMessageId(int groupMessageId) {
        groupMessageIds.emplace_back(groupMessageId, SYNC_DATA_PERSON_MESSAGE_DELETE);
    }

    void clearAllFriendIds() {
        friendIds.clear();
    }

    void clearAllPersonMessageIds() {
        personMessageIds.clear();
    }

    void clearAllGroupMessageIds() {
        groupMessageIds.clear();
    }

    static SyncState fromJson(const json& j) {
        SyncState state(j["user_id"].get<int>());
        state.friend_sync_state = j["friend_sync_state"].get<int>();
        state.person_message_sync_state = j["person_message_sync_state"].get<int>();
        state.group_message_sync_state = j["group_message_sync_state"].get<int>();
        state.friendIds = j["friendIds"].get<std::vector<std::pair<int, int>>>();
        state.personMessageIds = j["personMessageIds"].get<std::vector<std::pair<int, int>>>();
        state.groupMessageIds = j["groupMessageIds"].get<std::vector<std::pair<int, int>>>();
        return state;
    }

    json toJson() {
        json j;
        j["user_id"] = user_id;
        j["friend_sync_state"] = friend_sync_state;
        j["person_message_sync_state"] = person_message_sync_state;
        j["group_message_sync_state"] = group_message_sync_state;
        j["friendIds"] = friendIds;
        j["personMessageIds"] = personMessageIds;
        j["groupMessageIds"] = groupMessageIds;
        return j;
    }
};

struct File {
    int id{};
    std::string file_type;
    std::string file_name;
    std::string file_url;
    long file_size{};
    std::string file_md5;
    int user_id{};
    time_t timestamp{};

    File() = default;

    File(int id, const std::string& file_type, const std::string& file_name, const std::string& file_url,
         long file_size, const std::string& file_md5, int user_id, time_t timestamp) {
        this->id = id;
        this->file_type = file_type;
        this->file_name = file_name;
        this->file_url = file_url;
        this->file_size = file_size;
        this->file_md5 = file_md5;
        this->user_id = user_id;
        this->timestamp = timestamp;
    }

    File(const std::string& file_type, const std::string& file_name, const std::string& file_url, long file_size,
         const std::string& file_md5, int user_id, time_t timestamp) {
        this->file_type = file_type;
        this->file_name = file_name;
        this->file_url = file_url;
        this->file_size = file_size;
        this->file_md5 = file_md5;
        this->user_id = user_id;
        this->timestamp = timestamp;
    }
};

REFLECTION_WITH_NAME(File, "t_file", id, file_type, file_name, file_url, file_size, file_md5, user_id, timestamp)

struct ShardUploadStatus {
    int count{};
    std::unordered_set<int> shardIds;

    static ShardUploadStatus fromJson(const json& j) {
        ShardUploadStatus status;
        status.count = j["count"].get<int>();
        status.shardIds = j["shardIds"].get<std::unordered_set<int>>();
        return status;
    }

    json toJson() {
        json j;
        j["count"] = count;
        j["shardIds"] = shardIds;
        return j;
    }
};

struct LiveRoom {
    int id = 0;
    int owner_id = 0;

    LiveRoom() = default;

    explicit LiveRoom(int owner_id) {
        this->owner_id = owner_id;
    }
};

REFLECTION_WITH_NAME(LiveRoom, "t_live_room", id, owner_id)

struct LiveStatus {
    std::string cover;
    int viewer_count = 0;

    LiveStatus() = default;

    explicit LiveStatus(const std::string& cover) {
        this->cover = cover;
    }

    static LiveStatus fromJson(const json& j) {
        LiveStatus status;
        status.cover = j["cover"].get<std::string>();
        status.viewer_count = j["viewer_count"].get<int>();
        return status;
    }

    json toJson() {
        json j;
        j["cover"] = cover;
        j["viewer_count"] = viewer_count;
        return j;
    }
};

struct LiveRoomDTO {
    int id = 0;
    int owner_id = 0;
    std::string owner_nickname;
    std::string owner_avatar;
    std::string name;
    std::string cover;
    int viewer_count = 0;

    LiveRoomDTO() = default;

    LiveRoomDTO(int id, int owner_id, const std::string& owner_nickname, const std::string& owner_avatar,
                const std::string& name, const std::string& cover, int viewer_count) {
        this->id = id;
        this->owner_id = owner_id;
        this->owner_nickname = owner_nickname;
        this->owner_avatar = owner_avatar;
        this->name = name;
        this->cover = cover;
        this->viewer_count = viewer_count;
    }

    static LiveRoomDTO fromJson(const json& j) {
        LiveRoomDTO dto;
        dto.id = j["id"].get<int>();
        dto.owner_id = j["owner_id"].get<int>();
        dto.owner_nickname = j["owner_nickname"].get<std::string>();
        dto.owner_avatar = j["owner_avatar"].get<std::string>();
        dto.name = j["name"].get<std::string>();
        dto.cover = j["cover"].get<std::string>();
        dto.viewer_count = j["viewer_count"].get<int>();
        return dto;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["owner_id"] = owner_id;
        j["owner_nickname"] = owner_nickname;
        j["owner_avatar"] = owner_avatar;
        j["name"] = name;
        j["cover"] = cover;
        j["viewer_count"] = viewer_count;
        return j;
    }
};

REFLECTION(LiveRoomDTO, id, owner_id, owner_nickname, owner_avatar, name, cover, viewer_count)

struct Pyq {
    int id = 0;
    int user_id = 0;
    std::string content;
    // 存数组json序列化后的字符串
    std::string image_urls;
    time_t timestamp = 0;

    Pyq() = default;

    Pyq(int id, int user_id, const std::string& content, const std::string& image_urls, time_t timestamp) {
        this->id = id;
        this->user_id = user_id;
        this->content = content;
        this->image_urls = image_urls;
        this->timestamp = timestamp;
    }

    Pyq(int user_id, const std::string& content, const std::string& image_urls, time_t timestamp) {
        this->user_id = user_id;
        this->content = content;
        this->image_urls = image_urls;
        this->timestamp = timestamp;
    }

    static Pyq fromJson(const json& j) {
        Pyq pyq;
        pyq.id = j["id"].get<int>();
        pyq.user_id = j["user_id"].get<int>();
        pyq.content = j["content"].get<std::string>();
        pyq.image_urls = j["image_urls"].get<std::string>();
        pyq.timestamp = j["timestamp"].get<time_t>();
        return pyq;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["user_id"] = user_id;
        j["content"] = content;
        j["image_urls"] = image_urls;
        j["timestamp"] = timestamp;
        return j;
    }
};

REFLECTION_WITH_NAME(Pyq, "t_pyq", id, user_id, content, image_urls, timestamp)

struct PyqComment {
    int id = 0;
    int pyq_id = 0;
    // 1: 普通 2: 回复
    int type = 1;
    int user_id = 0;
    int reply_user_id = 0;
    std::string content;
    time_t timestamp = 0;

    PyqComment() = default;

    PyqComment(int id, int pyq_id, int type, int user_id, int reply_user_id, const std::string& content,
               time_t timestamp) {
        this->id = id;
        this->pyq_id = pyq_id;
        this->type = type;
        this->user_id = user_id;
        this->reply_user_id = reply_user_id;
        this->content = content;
        this->timestamp = timestamp;
    }

    PyqComment(int pyq_id, int type, int user_id, int reply_user_id, const std::string& content, time_t timestamp) {
        this->pyq_id = pyq_id;
        this->type = type;
        this->user_id = user_id;
        this->reply_user_id = reply_user_id;
        this->content = content;
        this->timestamp = timestamp;
    }

    static PyqComment fromJson(const json& j) {
        PyqComment comment;
        comment.id = j["id"].get<int>();
        comment.pyq_id = j["pyq_id"].get<int>();
        comment.type = j["type"].get<int>();
        comment.user_id = j["user_id"].get<int>();
        comment.reply_user_id = j["reply_user_id"].get<int>();
        comment.content = j["content"].get<std::string>();
        comment.timestamp = j["timestamp"].get<time_t>();
        return comment;
    }

    json toJson() {
        json j;
        j["id"] = id;
        j["pyq_id"] = pyq_id;
        j["type"] = type;
        j["user_id"] = user_id;
        j["reply_user_id"] = reply_user_id;
        j["content"] = content;
        j["timestamp"] = timestamp;
        return j;
    }
};

REFLECTION_WITH_NAME(PyqComment, "t_pyq_comment", id, pyq_id, type, user_id, reply_user_id, content, timestamp)

#endif
