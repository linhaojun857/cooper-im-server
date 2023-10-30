#include "UserController.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

UserController::UserController(std::shared_ptr<dbng<mysql>> sqlConn, std::shared_ptr<Redis> redisConn)
    : sqlConn_(std::move(sqlConn)), redisConn_(std::move(redisConn)) {
}

void UserController::getVfCode(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
}

void UserController::userLogin(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::userLogin";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "username", "password")
    auto username = params["username"].get<std::string>();
    auto password = params["password"].get<std::string>();
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (!std::regex_match(username, regex)) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "请输入正确的手机号码")
    }
    std::vector<User> users = sqlConn_->query<User>("username=" + username);
    if (users.empty()) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "请先注册")
    }
    assert(users.size() == 1);
    if (users[0].password != password) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "密码错误")
    }
    j["token"] = JwtUtil::createToken(users[0].id);
    j["user"] = users[0].toJson();
    IMStore::getInstance()->addOnlineUser(users[0].id, users[0]);
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "登录成功")
}

void UserController::userRegister(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::userRegister";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "username", "password")
    auto username = params["username"].get<std::string>();
    auto password = params["password"].get<std::string>();
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (!std::regex_match(username, regex)) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "请输入正确的手机号码")
    }
    std::vector<User> users = sqlConn_->query<User>("username=" + username);
    if (!users.empty()) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "该手机号码已注册")
    }
    User user(0, username, "user_" + username, password, DEFAULT_USER_AVATAR, DEFAULT_USER_STATUS,
              DEFAULT_USER_FEELING);
    if (sqlConn_->insert(user) != 1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "注册失败")
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "注册成功")
}

void UserController::getSyncState(const HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "UserController::getSyncState";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto ret = redisConn_->get(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(userId));
    if (!ret.has_value()) {
        SyncState syncState(userId);
        redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(userId), syncState.toJson().dump());
        j = syncState.toJson();
    } else {
        j = json::parse(ret.value());
    }
    LOG_DEBUG << "UserController::getSyncState: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取同步状态成功")
}

void UserController::getAllFriends(const HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "UserController::getAllFriends";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto friends = sqlConn_->query<User>(
        "select user.* "
        "from (select * from friend where a_id = " +
            std::to_string(userId) +
            ") as t1 "
            "left join user on t1.b_id = user.id;",
        1);
    for (auto& f : friends) {
        j["friends"].push_back(f.toJson());
    }
    LOG_DEBUG << "UserController::getAllFriends: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取好友列表成功")
}

void UserController::getFriendsByIds(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::getFriendsByIds";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token", "friendIds")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto friendIds = params["friendIds"].get<std::vector<int>>();
    std::string sql = "select * from user where id in (";
    for (auto& id : friendIds) {
        sql += std::to_string(id) + ",";
    }
    sql.pop_back();
    sql += ")";
    auto friends = sqlConn_->query<User>(sql);
    for (auto& f : friends) {
        j["friends"].push_back(f.toJson());
    }
    LOG_DEBUG << "UserController::getFriendsByIds: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取好友列表成功")
}

void UserController::getSyncFriends(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::getSyncFriends";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto ret = redisConn_->lpop(REDIS_KEY_SYNC_FRIEND_ENTITY_PREFIX + std::to_string(userId));
    while (ret.has_value()) {
        j["friends"].push_back(json::parse(ret.value()));
        ret = redisConn_->lpop(REDIS_KEY_SYNC_FRIEND_ENTITY_PREFIX + std::to_string(userId));
    }
    if (!j.contains("friends")) {
        LOG_DEBUG << "没有需要同步的好友";
    }
    LOG_DEBUG << "UserController::getSyncFriends: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取同步好友成功")
}

void UserController::search(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::search";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "keyword", "token")
    auto keyword = params["keyword"].get<std::string>();
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::vector<std::tuple<int>> ret =
        sqlConn_->query<std::tuple<int>>("select b_id from friend where a_id = " + std::to_string(userId));
    std::set<int> friendIds;
    for (auto& item : ret) {
        friendIds.insert(std::get<0>(item));
    }
    std::vector<User> users;
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (std::regex_match(keyword, regex)) {
        users = sqlConn_->query<User>("username=" + keyword);
        if (!users.empty()) {
            for (auto& item : users) {
                if (item.id != userId && friendIds.find(item.id) == friendIds.end()) {
                    j["fsrs"].push_back(item.toJson());
                }
            }
        }
    }
    users.clear();
    users = sqlConn_->query<User>("nickname like '%" + keyword + "%'");
    if (!users.empty()) {
        for (auto& item : users) {
            if (item.id != userId && friendIds.find(item.id) == friendIds.end()) {
                j["fsrs"].push_back(item.toJson());
            }
        }
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "搜索成功")
}

void UserController::addFriend(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::addFriend";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "peerId", "reason", "token")
    int peerId = params["peerId"].get<int>();
    std::string reason = params["reason"].get<std::string>();
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    sqlConn_->begin();
    try {
        auto ret1 =
            sqlConn_->query<Friend>("a_id = " + std::to_string(userId) + " and b_id = " + std::to_string(peerId));
        if (!ret1.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复添加")
        }
        auto ret2 = sqlConn_->query<FriendApply>("from_id = " + std::to_string(userId) +
                                                 " and to_id = " + std::to_string(peerId) + " and agree = 0");
        if (!ret2.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复申请")
        }
        std::shared_ptr<User> user;
        if (IMStore::getInstance()->isOnlineUser(userId)) {
            user = IMStore::getInstance()->getOnlineUser(userId);
        } else {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "用户未登录")
        }
        auto peerUser = sqlConn_->query<User>("id = " + std::to_string(peerId));
        FriendApply fa(0, userId, peerId, user->avatar, user->nickname, peerUser[0].avatar, peerUser[0].nickname,
                       reason, 0);
        sqlConn_->insert(fa);
        auto id = sqlConn_->query<std::tuple<int>>("select LAST_INSERT_ID()");
        Notify notify(0, userId, 0, std::get<0>(id[0]));
        sqlConn_->insert(notify);
        notify.to_id = peerId;
        if (IMStore::getInstance()->haveTcpConnection(peerId)) {
            auto peerConnPtr = IMStore::getInstance()->getTcpConnection(peerId);
            json toPeerJson = fa.toJson();
            toPeerJson["type"] = PROTOCOL_TYPE_FRIEND_APPLY_NOTIFY_P;
            peerConnPtr->sendJson(toPeerJson);
        } else {
            json j1;
            j1["notify_id"] = notify.id;
            j1["notify_type"] = notify.notify_type;
            j1["data"] = fa.toJson();
            redisConn_->lpush(REDIS_KEY_NOTIFY_QUEUE_PREFIX + std::to_string(peerId), j1.dump());
        }
        sqlConn_->insert(notify);
    } catch (std::exception& e) {
        sqlConn_->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "添加好友失败")
    }
    sqlConn_->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "成功发送好友申请")
}

void UserController::responseFriendApply(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "UserController::responseFriendApply";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "from_id", "agree", "token")
    int from_id = params["from_id"].get<int>();
    int agree = params["agree"].get<int>();
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    sqlConn_->begin();
    try {
        auto fa = sqlConn_->query<FriendApply>("from_id = " + std::to_string(from_id) +
                                               " and to_id = " + std::to_string(userId) + " and agree = 0");
        if (fa.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "没有对应的好友申请")
        }
        if (fa[0].to_id != userId) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
        }
        if (fa[0].agree != 0) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复操作")
        }
        if (agree == 1) {
            Friend friend1(0, fa[0].from_id, fa[0].to_id, 0);
            Friend friend2(0, fa[0].to_id, fa[0].from_id, 0);
            sqlConn_->insert(friend1);
            sqlConn_->insert(friend2);
        }
        fa[0].agree = agree;
        sqlConn_->update(fa[0]);
        Notify notify(0, userId, 0, fa[0].id);
        sqlConn_->insert(notify);
        notify.to_id = fa[0].from_id;
        if (!IMStore::getInstance()->haveTcpConnection(userId)) {
            sqlConn_->rollback();
            RETURN_RESPONSE(HTTP_ERROR_CODE, "用户未登录")
        }
        if (fa[0].agree == 1) {
            auto user = sqlConn_->query<User>("id = " + std::to_string(fa[0].from_id));
            json j1 = user[0].toJson();
            j1["type"] = PROTOCOL_TYPE_FRIEND_ENTITY;
            IMStore::getInstance()->getTcpConnection(userId)->sendJson(j1);
        }
        if (IMStore::getInstance()->haveTcpConnection(fa[0].from_id)) {
            auto connPtr = IMStore::getInstance()->getTcpConnection(fa[0].from_id);
            json toUserJson = fa[0].toJson();
            toUserJson["type"] = PROTOCOL_TYPE_FRIEND_APPLY_NOTIFY_I;
            connPtr->sendJson(toUserJson);
            if (fa[0].agree == 1) {
                json j2 = IMStore::getInstance()->getOnlineUser(userId)->toJson();
                j2["type"] = PROTOCOL_TYPE_FRIEND_ENTITY;
                connPtr->sendJson(j2);
            }
        } else {
            json j2;
            j2["notify_id"] = notify.id;
            j2["notify_type"] = notify.notify_type;
            j2["data"] = fa[0].toJson();
            redisConn_->lpush(REDIS_KEY_NOTIFY_QUEUE_PREFIX + std::to_string(fa[0].from_id), j2.dump());
            if (fa[0].agree == 1) {
                auto ret = redisConn_->get(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(fa[0].from_id));
                if (!ret.has_value()) {
                    SyncState syncState(fa[0].from_id);
                    syncState.friend_sync_state = 1;
                    syncState.addInsertedFriendId(userId);
                    redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(fa[0].from_id),
                                    syncState.toJson().dump());
                } else {
                    SyncState syncState = SyncState::fromJson(json::parse(ret.value()));
                    syncState.friend_sync_state = 1;
                    syncState.addInsertedFriendId(userId);
                    redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(fa[0].from_id),
                                    syncState.toJson().dump());
                }
                redisConn_->lpush(REDIS_KEY_SYNC_FRIEND_ENTITY_PREFIX + std::to_string(fa[0].from_id),
                                  IMStore::getInstance()->getOnlineUser(userId)->toJson().dump());
            }
        }
        sqlConn_->insert(notify);
    } catch (std::exception& e) {
        sqlConn_->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "回应FriendApply失败")
    }
    sqlConn_->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "回应FriendApply成功")
}

void UserController::handleAuthMsg(const cooper::TcpConnectionPtr& connPtr, const nlohmann::json& params) {
    LOG_DEBUG << "UserController::handleAuthMsg";
    void(this);
    if (!params.contains("token")) {
        RETURN_ERROR("缺少token")
    }
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("无效token")
    }
    IMStore::getInstance()->addTcpConnection(userId, connPtr);
}

void UserController::handleSyncCompleteMsg(const TcpConnectionPtr& connPtr, const json& params) {
    LOG_DEBUG << "UserController::handleSyncCompleteMsg";
    TCP_CHECK_PARAMS(params, "token")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("无效token")
    }
    auto ret = redisConn_->get(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(userId));
    if (!ret.has_value()) {
        LOG_ERROR << "UserController::handleSyncCompleteMsg: "
                  << "获取同步状态失败";
        SyncState syncState(userId);
        redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(userId), syncState.toJson().dump());
    } else {
        SyncState syncState = SyncState::fromJson(json::parse(ret.value()));
        syncState.friend_sync_state = 0;
        syncState.person_message_sync_state = 0;
        syncState.clearAllFriendIds();
        syncState.clearAllPersonMessageIds();
        redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(userId), syncState.toJson().dump());
    }
}
