#include "PyqController.hpp"

#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

PyqController::PyqController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
    IMStore::getInstance()->registerPyqController(this);
}

void PyqController::postPyq(HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "PyqController::postPyq";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "content", "image_urls")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::string content = params["content"].get<std::string>();
    std::string image_urls = params["image_urls"].get<std::string>();
    GET_SQL_CONN_H(sqlConn)
    Pyq pyq;
    pyq.user_id = userId;
    pyq.content = content;
    pyq.image_urls = image_urls;
    pyq.timestamp = time(nullptr);
    auto ret = sqlConn->insert(pyq);
    if (ret) {
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "发布成功")
    } else {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "发布失败")
    }
}

void PyqController::getPyq(HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "PyqController::getPyq";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    std::vector<int> friendIds;
    auto temps = sqlConn->query<std::tuple<int>>("select b_id from t_friend where a_id =" + std::to_string(userId));
    friendIds.reserve(temps.size());
    for (const auto& temp : temps) {
        friendIds.emplace_back(std::get<0>(temp));
    }
    friendIds.emplace_back(userId);
    std::string sql = "select * from t_pyq where user_id in (";
    for (int i = 0; i < friendIds.size(); ++i) {
        sql += std::to_string(friendIds[i]);
        if (i != friendIds.size() - 1) {
            sql += ",";
        }
    }
    sql += ") order by timestamp desc";
    auto pyqs = sqlConn->query<Pyq>(sql);
    for (auto& pyq : pyqs) {
        j["pyqs"].push_back(pyq.toJson());
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取朋友圈成功")
}
