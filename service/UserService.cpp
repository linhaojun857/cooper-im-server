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
    // TODO: 对接腾讯云短信服务
}

void UserService::userLogin(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    int code = HTTP_SUCCESS_CODE;
    std::string msg;
    auto params = json::parse(request.body_);
    auto username = params["username"].get<std::string>();
    auto password = params["password"].get<std::string>();
    // auto vfCode = params["vfCode"].get<std::string>();
    std::vector<User> users;
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    json j;
    std::vector<User> friends;
    std::string token;
    if (!std::regex_match(username, regex)) {
        code = HTTP_ERROR_CODE;
        msg = "请输入正确的手机号码";
        goto last;
    }
    // TODO: 校验验证码
    users = sqlConn_->query<User>("username=" + username);
    if (users.empty()) {
        code = HTTP_ERROR_CODE;
        msg = "请先注册";
        goto last;
    }
    assert(users.size() == 1);
    if (users[0].password != password) {
        code = HTTP_ERROR_CODE;
        msg = "密码错误";
        goto last;
    }
    code = HTTP_SUCCESS_CODE;
    msg = "登录成功";
    j["token"] = JwtUtil::createToken(users[0].id);
    j["user"] = users[0].toJson();
    IMStore::getInstance()->addOnlineUser(users[0].id, users[0]);
    friends = sqlConn_->query<User>(
        "select user.* "
        "from (select * from friend where a_id = " +
            std::to_string(users[0].id) +
            ") as t1 "
            "left join user on t1.b_id = user.id;",
        1);
    for (auto& f : friends) {
        j["friends"].push_back(f.toJson());
    }
last:
    j["code"] = code;
    j["msg"] = msg;
    response.body_ = j.dump();
}

void UserService::userRegister(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    int code;
    std::string msg;
    auto params = json::parse(request.body_);
    auto username = params["username"].get<std::string>();
    auto password = params["password"].get<std::string>();
    // auto vfCode = params["vfCode"].get<std::string>();
    std::vector<User> users;
    User user;
    int ret;
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (!std::regex_match(username, regex)) {
        code = HTTP_ERROR_CODE;
        msg = "请输入正确的手机号码";
        goto last;
    }
    // TODO: 校验验证码
    users = sqlConn_->query<User>("username=" + username);
    if (!users.empty()) {
        code = HTTP_ERROR_CODE;
        msg = "该手机号码已注册";
        goto last;
    }
    user.username = username;
    user.nickname = "user_" + username;
    user.password = password;
    user.avatar = DEFAULT_USER_AVATAR;
    user.status = DEFAULT_USER_STATUS;
    user.feeling = DEFAULT_USER_FEELING;
    ret = sqlConn_->insert(user);
    if (ret != 1) {
        code = HTTP_ERROR_CODE;
        msg = "注册失败";
        goto last;
    } else {
        code = HTTP_SUCCESS_CODE;
        msg = "注册成功";
        goto last;
    }
last:
    json j;
    j["code"] = code;
    j["msg"] = msg;
    response.body_ = j.dump();
}

void UserService::search(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    int code;
    std::string msg;
    auto params = json::parse(request.body_);
    auto keyword = params["keyword"].get<std::string>();
    std::string token;
    int userId;
    std::vector<User> users;
    json j;
    std::regex regex(R"(^1(3\d|4[5-9]|5[0-35-9]|6[2567]|7[0-8]|8\d|9[0-35-9])\d{8}$)");
    if (!params.contains("token")) {
        code = HTTP_ERROR_CODE;
        msg = "缺少token";
        goto last;
    }
    token = params["token"].get<std::string>();
    userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        code = HTTP_ERROR_CODE;
        msg = "无效token";
        goto last;
    }
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
    code = HTTP_SUCCESS_CODE;
    msg = "搜索成功";
last:
    j["code"] = code;
    j["msg"] = msg;
    response.body_ = j.dump();
}

void UserService::addFriend(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    int code;
    std::string msg;
    auto params = json::parse(request.body_);
    json j;
    std::vector<Friend> ret1;
    std::vector<FriendApply> ret2;
    int userId;
    int peerId;
    std::string reason;
    std::string token;
    FriendApply fa{};
    Notify n{};
    User user;
    std::vector<std::tuple<int>> id;
    if (!params.contains("peerId")) {
        code = HTTP_ERROR_CODE;
        msg = "缺少peerId";
        goto last;
    }
    peerId = params["peerId"].get<int>();
    if (!params.contains("reason")) {
        code = HTTP_ERROR_CODE;
        msg = "缺少reason";
        goto last;
    }
    reason = params["reason"].get<std::string>();
    if (!params.contains("token")) {
        code = HTTP_ERROR_CODE;
        msg = "缺少token";
        goto last;
    }
    token = params["token"].get<std::string>();
    userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        code = HTTP_ERROR_CODE;
        msg = "无效token";
        goto last;
    }
    sqlConn_->begin();
    try {
        ret1 = sqlConn_->query<Friend>("a_id = " + std::to_string(userId) + " and b_id = " + std::to_string(peerId));
        if (!ret1.empty()) {
            code = HTTP_ERROR_CODE;
            msg = "请勿重复添加";
            goto last;
        }
        ret2 = sqlConn_->query<FriendApply>("from_id = " + std::to_string(userId) +
                                            " and to_id = " + std::to_string(peerId) + " and agree = 0");
        if (!ret2.empty()) {
            code = HTTP_ERROR_CODE;
            msg = "请勿重复申请";
            goto last;
        }
        if (IMStore::getInstance()->isOnlineUser(userId)) {
            user = IMStore::getInstance()->getOnlineUser(userId);
        } else {
            code = HTTP_ERROR_CODE;
            msg = "用户不在线";
            goto last;
        }
        fa.from_id = userId;
        fa.to_id = peerId;
        fa.avatar = user.avatar;
        fa.nickname = user.nickname;
        fa.reason = reason;
        fa.agree = 0;
        sqlConn_->insert(fa);
        id = sqlConn_->query<std::tuple<int>>("select LAST_INSERT_ID()");
        n.type = 0;
        n.to_id = userId;
        n.fa_id = std::get<0>(id[0]);
        n.is_complete = 0;
        sqlConn_->insert(n);
        n.to_id = peerId;
        sqlConn_->insert(n);
        code = HTTP_SUCCESS_CODE;
        msg = "成功发送好友申请";
    } catch (std::exception& e) {
        sqlConn_->rollback();
        LOG_ERROR << e.what();
        code = HTTP_ERROR_CODE;
        msg = "添加好友失败";
        goto last;
    }
    sqlConn_->commit();
last:
    j["code"] = code;
    j["msg"] = msg;
    response.body_ = j.dump();
}
