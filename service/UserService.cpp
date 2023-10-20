#include "UserService.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

UserService::UserService(std::shared_ptr<dbng<mysql>> sqlConn) : sqlConn_(std::move(sqlConn)) {
}

void UserService::getVfCode(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
}

void UserService::userLogin(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    json j;
    auto params = json::parse(request.body_);
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
    std::vector<User> friends = sqlConn_->query<User>(
        "select user.* "
        "from (select * from friend where a_id = " +
            std::to_string(users[0].id) +
            ") as t1 "
            "left join user on t1.b_id = user.id;",
        1);
    for (auto& f : friends) {
        j["friends"].push_back(f.toJson());
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "登录成功")
}

void UserService::userRegister(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    json j;
    auto params = json::parse(request.body_);
    auto username = params["username"].get<std::string>();
    auto password = params["password"].get<std::string>();
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (!std::regex_match(username, regex)) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "请输入正确的手机号码");
    }
    std::vector<User> users = sqlConn_->query<User>("username=" + username);
    if (!users.empty()) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "该手机号码已注册");
    }
    User user(0, username, "user_" + username, password, DEFAULT_USER_AVATAR, DEFAULT_USER_STATUS,
              DEFAULT_USER_FEELING);
    if (sqlConn_->insert(user) != 1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "注册失败")
    } else {
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "注册成功")
    }
}

void UserService::search(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    json j;
    auto params = json::parse(request.body_);
    auto keyword = params["keyword"].get<std::string>();
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (!params.contains("token")) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "缺少token")
    }
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::vector<User> users;
    if (std::regex_match(keyword, regex)) {
        users = sqlConn_->query<User>("username=" + keyword);
        if (!users.empty()) {
            for (auto& item : users) {
                if (item.id != userId) {
                    j["fsrs"].push_back(item.toJson());
                }
            }
        }
    }
    users.clear();
    users = sqlConn_->query<User>("nickname like '%" + keyword + "%'");
    if (!users.empty()) {
        for (auto& item : users) {
            if (item.id != userId) {
                j["fsrs"].push_back(item.toJson());
            }
        }
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "搜索成功");
}

void UserService::addFriend(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    auto params = json::parse(request.body_);
    json j;
    if (!params.contains("peerId")) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "缺少peerId")
    }
    int peerId = params["peerId"].get<int>();
    if (!params.contains("reason")) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "缺少reason")
    }
    std::string reason = params["reason"].get<std::string>();
    if (!params.contains("token")) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "缺少token")
    }
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
        User user;
        if (IMStore::getInstance()->isOnlineUser(userId)) {
            user = IMStore::getInstance()->getOnlineUser(userId);
        } else {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "用户不在线")
        }
        FriendApply fa(0, userId, peerId, user.avatar, user.nickname, reason, 0);
        sqlConn_->insert(fa);
        auto id = sqlConn_->query<std::tuple<int>>("select LAST_INSERT_ID()");
        Notify notify(0, 0, userId, std::get<0>(id[0]), 0);
        sqlConn_->insert(notify);
        notify.to_id = peerId;
        sqlConn_->insert(notify);
    } catch (std::exception& e) {
        sqlConn_->rollback();
        LOG_ERROR << e.what();
        RETURN_RESPONSE(HTTP_ERROR_CODE, "添加好友失败")
    }
    sqlConn_->commit();
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "成功发送好友申请")
}
