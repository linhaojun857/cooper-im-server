#include "FriendController.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"
#include "util/JwtUtil.hpp"

FriendController::FriendController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
    IMStore::getInstance()->registerFriendController(this);
}

void FriendController::getAllFriends(HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "FriendController::getAllFriends";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    auto friends = sqlConn->query<UserDTO>(
        "select t_user.*, t1.session_id "
        "from (select * from t_friend where a_id = " +
        std::to_string(userId) +
        ") as t1 "
        "left join t_user on t1.b_id = t_user.id;");
    for (auto& f : friends) {
        j["friends"].push_back(f.toJson());
    }
    LOG_DEBUG << "FriendController::getAllFriends: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取好友列表成功")
}

void FriendController::getFriendsByIds(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FriendController::getFriendsByIds";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token", "friendIds")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto friendIds = params["friendIds"].get<std::vector<int>>();
    std::string sql = "select * from t_user where id in (";
    for (auto& id : friendIds) {
        sql += std::to_string(id) + ",";
    }
    sql.pop_back();
    sql += ")";
    GET_SQL_CONN_H(sqlConn)
    auto friends = sqlConn->query<User>(sql);
    for (auto& f : friends) {
        j["friends"].push_back(f.toJson());
    }
    LOG_DEBUG << "FriendController::getFriendsByIds: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取好友列表成功")
}

void FriendController::getSyncFriends(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FriendController::getSyncFriends";
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
    LOG_DEBUG << "FriendController::getSyncFriends: " << j.dump();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取同步好友成功")
}

void FriendController::searchFriend(HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "FriendController::searchFriend";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token", "keyword")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto keyword = params["keyword"].get<std::string>();
    GET_SQL_CONN_H(sqlConn)
    auto ret = sqlConn->query<std::tuple<int>>("select b_id from t_friend where a_id = " + std::to_string(userId));
    std::set<int> friendIds;
    for (auto& item : ret) {
        friendIds.insert(std::get<0>(item));
    }
    std::vector<User> users;
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (std::regex_match(keyword, regex)) {
        users = sqlConn->query<User>("username=" + keyword);
        if (!users.empty()) {
            for (auto& item : users) {
                if (item.id != userId && friendIds.find(item.id) == friendIds.end()) {
                    j["fsrs"].push_back(item.toJson());
                }
            }
        }
    }
    users.clear();
    users = sqlConn->query<User>("nickname like '%" + keyword + "%'");
    if (!users.empty()) {
        for (auto& item : users) {
            if (item.id != userId && friendIds.find(item.id) == friendIds.end()) {
                j["fsrs"].push_back(item.toJson());
            }
        }
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "搜索成功")
}

void FriendController::addFriend(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FriendController::addFriend";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "peerId", "reason")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    int peerId = params["peerId"].get<int>();
    std::string reason = params["reason"].get<std::string>();
    GET_SQL_CONN_H(sqlConn)
    sqlConn->begin();
    try {
        auto ret1 =
            sqlConn->query<Friend>("a_id = " + std::to_string(userId) + " and b_id = " + std::to_string(peerId));
        if (!ret1.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复添加")
        }
        auto ret2 = sqlConn->query<FriendApply>("from_id = " + std::to_string(userId) +
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
        auto peerUser = sqlConn->query<User>("id = " + std::to_string(peerId));
        FriendApply fa(0, userId, peerId, user->avatar, user->nickname, peerUser[0].avatar, peerUser[0].nickname,
                       reason, 0);
        sqlConn->insert(fa);
        auto id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()");
        fa.id = std::get<0>(id[0]);
        Notify notify(0, userId, 0, std::get<0>(id[0]));
        sqlConn->insert(notify);
        notify.to_id = peerId;
        sqlConn->insert(notify);
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
    } catch (std::exception& e) {
        sqlConn->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "添加好友失败")
    }
    sqlConn->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "成功发送好友申请")
}

void FriendController::responseFriendApply(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FriendController::responseFriendApply";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "apply_id", "agree")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    int apply_id = params["apply_id"].get<int>();
    int agree = params["agree"].get<int>();
    GET_SQL_CONN_H(sqlConn)
    sqlConn->begin();
    try {
        auto fa = sqlConn->query<FriendApply>("id = " + std::to_string(apply_id));
        if (fa.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "没有对应的好友申请")
        }
        if (fa[0].to_id != userId) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
        }
        if (fa[0].agree != 0) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复操作")
        }
        std::string session_id = IMUtil::generateUUid();
        if (agree == 1) {
            Friend friend1(0, fa[0].from_id, fa[0].to_id, session_id, 0);
            Friend friend2(0, fa[0].to_id, fa[0].from_id, session_id, 0);
            sqlConn->insert(friend1);
            sqlConn->insert(friend2);
        }
        fa[0].agree = agree;
        sqlConn->update(fa[0]);
        Notify notify(0, userId, 0, fa[0].id);
        sqlConn->insert(notify);
        notify.to_id = fa[0].from_id;
        sqlConn->insert(notify);
        if (!IMStore::getInstance()->haveTcpConnection(userId)) {
            sqlConn->rollback();
            RETURN_RESPONSE(HTTP_ERROR_CODE, "用户未登录")
        }
        if (fa[0].agree == 1) {
            auto user = sqlConn->query<User>("id = " + std::to_string(fa[0].from_id));
            json j1 = user[0].toJson();
            j1["type"] = PROTOCOL_TYPE_FRIEND_ENTITY;
            j1["status"] = SYNC_DATA_FRIEND_ENTITY_INSERT;
            j1["session_id"] = session_id;
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
                j2["status"] = SYNC_DATA_FRIEND_ENTITY_INSERT;
                j2["session_id"] = session_id;
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
                auto j3 = IMStore::getInstance()->getOnlineUser(userId)->toJson();
                j3["session_id"] = session_id;
                redisConn_->lpush(REDIS_KEY_SYNC_FRIEND_ENTITY_PREFIX + std::to_string(fa[0].from_id),
                                  IMStore::getInstance()->getOnlineUser(userId)->toJson().dump());
            }
        }
    } catch (std::exception& e) {
        sqlConn->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "回应FriendApply失败")
    }
    sqlConn->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "回应FriendApply成功")
}
