#include "UserController.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"
#include "util/JwtUtil.hpp"

UserController::UserController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
    IMStore::getInstance()->registerUserController(this);
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
    GET_SQL_CONN_H(sqlConn)
    std::vector<User> users = sqlConn->query<User>("username=" + username);
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
    GET_SQL_CONN_H(sqlConn)
    std::vector<User> users = sqlConn->query<User>("username=" + username);
    if (!users.empty()) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "该手机号码已注册")
    }
    User user(0, username, "user_" + username, password, DEFAULT_USER_AVATAR, DEFAULT_USER_STATUS,
              DEFAULT_USER_FEELING);
    if (sqlConn->insert(user) != 1) {
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
        syncState.group_message_sync_state = 0;
        syncState.clearAllFriendIds();
        syncState.clearAllPersonMessageIds();
        syncState.clearAllGroupMessageIds();
        redisConn_->set(REDIS_KEY_SYNC_STATE_PREFIX + std::to_string(userId), syncState.toJson().dump());
    }
    redisConn_->del(REDIS_KEY_PERSON_OFFLINE_MSG + std::to_string(userId));
}
