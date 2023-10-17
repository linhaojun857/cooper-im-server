#include "UserService.hpp"

#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"

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
    if (users[0].password == password) {
        code = HTTP_SUCCESS_CODE;
        msg = "登录成功";
        goto last;
    }
last:
    IMStore::addOnlineUser(users[0].id);
    json j;
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
