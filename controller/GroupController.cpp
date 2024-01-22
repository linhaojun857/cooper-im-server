#include "GroupController.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"
#include "util/JwtUtil.hpp"

GroupController::GroupController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
    IMStore::getInstance()->registerGroupController(this);
}

void GroupController::createGroup(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "GroupController::createGroup";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "name", "desc")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::string name = params["name"].get<std::string>();
    std::string desc = params["desc"].get<std::string>();
    std::string session_id = IMUtil::generateUUid();
    std::string group_num = IMUtil::generateGroupNum();
    Group group(session_id, group_num, userId, name, DEFAULT_GROUP_AVATAR, desc);
    GET_SQL_CONN_H(sqlConn)
    sqlConn->begin();
    try {
        sqlConn->insert(group);
        auto id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()");
        UserGroup userGroup(userId, std::get<0>(id[0]));
        sqlConn->insert(userGroup);
    } catch (const std::exception& e) {
        sqlConn->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "创建失败")
    }
    sqlConn->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "创建成功")
}

void GroupController::searchGroup(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "GroupController::searchGroup";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "keyword")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::string keyword = params["keyword"].get<std::string>();
    GET_SQL_CONN_H(sqlConn)
    auto ret =
        sqlConn->query<std::tuple<int>>("select group_id from t_user_group where user_id = " + std::to_string(userId));
    std::set<int> groupIds;
    for (const auto& item : ret) {
        groupIds.insert(std::get<0>(item));
    }
    std::regex regex(R"(^\d{10}$)");
    if (std::regex_match(keyword, regex)) {
        auto groups = sqlConn->query<Group>("group_num=" + keyword);
        if (!groups.empty()) {
            for (auto& item : groups) {
                if (groupIds.find(item.id) == groupIds.end()) {
                    j["gsrs"].push_back(item.toJson());
                }
            }
        }
    }
    auto groups = sqlConn->query<Group>("name like '%" + keyword + "%'");
    if (!groups.empty()) {
        for (auto& item : groups) {
            if (groupIds.find(item.id) == groupIds.end()) {
                j["gsrs"].push_back(item.toJson());
            }
        }
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "搜索成功")
}

void GroupController::addGroup(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "GroupController::addGroup";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "reason", "group_id")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::string reason = params["reason"].get<std::string>();
    int group_id = params["group_id"].get<int>();
    GET_SQL_CONN_H(sqlConn)
    sqlConn->begin();
    try {
        auto ret1 = sqlConn->query<Group>("id = " + std::to_string(group_id));
        if (ret1.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "群不存在")
        }
        auto ret2 = sqlConn->query<UserGroup>("user_id = " + std::to_string(userId) +
                                              " and group_id = " + std::to_string(group_id));
        if (!ret2.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复加入")
        }
        auto ret3 =
            sqlConn->query<std::tuple<int>>("select owner_id from t_group where id = " + std::to_string(group_id));
        int ownerId = std::get<0>(ret3[0]);
        auto ret4 = sqlConn->query<GroupApply>("from_id = " + std::to_string(userId) +
                                               " and to_id = " + std::to_string(ownerId) +
                                               " and group_id = " + std::to_string(group_id) + " and agree = 0");
        if (!ret4.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复申请")
        }
        std::shared_ptr<User> user;
        if (IMStore::getInstance()->isOnlineUser(userId)) {
            user = IMStore::getInstance()->getOnlineUser(userId);
        } else {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "用户未登录")
        }
        if (userId == ownerId) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "你已在群内")
        }
        auto group = sqlConn->query<Group>("id = " + std::to_string(group_id));
        GroupApply ga(0, userId, ownerId, group_id, user->avatar, user->nickname, group[0].avatar, group[0].name,
                      reason, 0);
        sqlConn->insert(ga);
        auto id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()");
        ga.id = std::get<0>(id[0]);
        Notify notify(0, userId, 1, std::get<0>(id[0]));
        sqlConn->insert(notify);
        notify.to_id = ownerId;
        sqlConn->insert(notify);
        if (IMStore::getInstance()->haveTcpConnection(ownerId)) {
            auto ownerConnPtr = IMStore::getInstance()->getTcpConnection(ownerId);
            json toOwnerJson = ga.toJson();
            toOwnerJson["type"] = PROTOCOL_TYPE_GROUP_APPLY_NOTIFY_P;
            ownerConnPtr->sendJson(toOwnerJson);
        } else {
            json j1;
            j1["notify_id"] = notify.id;
            j1["notify_type"] = notify.notify_type;
            j1["data"] = ga.toJson();
            redisConn_->lpush(REDIS_KEY_NOTIFY_QUEUE_PREFIX + std::to_string(ownerId), j1.dump());
        }
    } catch (std::exception& e) {
        sqlConn->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "添加群失败")
    }
    sqlConn->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "成功发送群申请")
}

void GroupController::responseGroupApply(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "GroupController::responseGroupApply";
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
        auto ga = sqlConn->query<GroupApply>("id = " + std::to_string(apply_id));
        if (ga.empty()) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "没有对应的群申请")
        }
        if (ga[0].to_id != userId) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
        }
        if (ga[0].agree != 0) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "请勿重复操作")
        }
        if (agree == 1) {
            UserGroup userGroup(ga[0].from_id, ga[0].group_id);
            sqlConn->insert(userGroup);
        }
        ga[0].agree = agree;
        sqlConn->update(ga[0]);
        Notify notify(0, userId, 1, ga[0].id);
        sqlConn->insert(notify);
        notify.to_id = ga[0].from_id;
        sqlConn->insert(notify);
        if (IMStore::getInstance()->haveTcpConnection(ga[0].from_id)) {
            auto connPtr = IMStore::getInstance()->getTcpConnection(ga[0].from_id);
            json toUserJson = ga[0].toJson();
            toUserJson["type"] = PROTOCOL_TYPE_GROUP_APPLY_NOTIFY_I;
            connPtr->sendJson(toUserJson);
            if (ga[0].agree == 1) {
                auto group = sqlConn->query<Group>("id = " + std::to_string(ga[0].group_id));
                json j1 = group[0].toJson();
                j1["type"] = PROTOCOL_TYPE_GROUP_ENTITY;
                connPtr->sendJson(j1);
            }
        } else {
            json j2;
            j2["notify_id"] = notify.id;
            j2["notify_type"] = notify.notify_type;
            j2["data"] = ga[0].toJson();
            redisConn_->lpush(REDIS_KEY_NOTIFY_QUEUE_PREFIX + std::to_string(ga[0].from_id), j2.dump());
        }
    } catch (const std::exception& e) {
        sqlConn->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "回应GroupApply失败")
    }
    sqlConn->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "回应GroupApply成功")
}

void GroupController::getAllGroups(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "GroupController::getAllGroups";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    auto groups = sqlConn->query<Group>(
        "select * from t_group where id in (select group_id from t_user_group where "
        "user_id = " +
        std::to_string(userId) + ")");
    for (auto& group : groups) {
        j["groups"].push_back(group.toJson());
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取群列表成功")
}
