#include "LiveController.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"
#include "util/JwtUtil.hpp"

LiveController::LiveController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
    IMStore::getInstance()->registerLiveController(this);
}

void LiveController::openLive(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "LiveController::openLive";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "cover")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    std::string cover = params["cover"].get<std::string>();
    GET_SQL_CONN_H(sqlConn)
    LiveStatus status(cover);
    auto ret = sqlConn->query<std::tuple<int, std::string>>("select id from t_live_room where owner_id = " +
                                                            std::to_string(userId));
    if (!ret.empty()) {
        int room_id = std::get<0>(ret[0]);
        j["room_id"] = room_id;
        redisConn_->set(REDIS_KEY_LIVE_ROOM + std::to_string(room_id), status.toJson().dump());
        redisConn_->sadd(REDIS_KEY_LIVE_ROOM_SET, std::to_string(room_id));
        redisConn_->set(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId), std::to_string(room_id));
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "开启成功")
    }
    LiveRoom liveRoom(userId);
    sqlConn->insert(liveRoom);
    auto id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()");
    int room_id = std::get<0>(id[0]);
    j["room_id"] = room_id;
    redisConn_->set(REDIS_KEY_LIVE_ROOM + std::to_string(room_id), status.toJson().dump());
    redisConn_->sadd(REDIS_KEY_LIVE_ROOM_SET, std::to_string(room_id));
    redisConn_->set(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId), std::to_string(room_id));
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "开启成功")
}

void LiveController::closeLive(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "LiveController::closeLive";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "room_id")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    int room_id = params["room_id"].get<int>();
    redisConn_->del(REDIS_KEY_LIVE_ROOM + std::to_string(room_id));
    redisConn_->srem(REDIS_KEY_LIVE_ROOM_SET, std::to_string(room_id));
}

void LiveController::getOpenedLives(const HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "LiveController::getOpenedLives";
    json j;
    GET_SQL_CONN_H(sqlConn)
    std::vector<std::string> openedLiveRoomIds;
    redisConn_->smembers(REDIS_KEY_LIVE_ROOM_SET, std::back_inserter(openedLiveRoomIds));
    if (openedLiveRoomIds.empty()) {
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
    }
    for (const auto& liveRoomId : openedLiveRoomIds) {
        auto ret1 = redisConn_->get(REDIS_KEY_LIVE_ROOM + liveRoomId);
        if (ret1.has_value()) {
            auto status = LiveStatus::fromJson(json::parse(ret1.value()));
            auto ret2 = sqlConn->query<LiveRoomDTO>(
                "select "
                "t_live_room.id, "
                "t_user.id, "
                "t_user.nickname as owner_nickname, "
                "t_user.avatar as owner_avatar "
                "from t_live_room left join t_user "
                "on t_live_room.owner_id = "
                "t_user.id where t_live_room.id = " +
                liveRoomId);
            if (!ret2.empty()) {
                auto jRet = ret2[0].toJson();
                jRet["cover"] = status.cover;
                jRet["viewer_count"] = status.viewer_count;
                j["live_rooms"].push_back(jRet);
            }
        }
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}

void LiveController::getOpenedLiveInfoByRoomId(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "LiveController::getOpenedLiveInfoByRoomId";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "room_id")
    int room_id = params["room_id"].get<int>();
    GET_SQL_CONN_H(sqlConn)
    auto ret1 = redisConn_->get(REDIS_KEY_LIVE_ROOM + std::to_string(room_id));
    if (ret1.has_value()) {
        auto status = LiveStatus::fromJson(json::parse(ret1.value()));
        auto ret2 = sqlConn->query<LiveRoomDTO>(
            "select "
            "t_live_room.id, "
            "t_user.id, "
            "t_user.nickname as owner_nickname, "
            "t_user.avatar as owner_avatar "
            "from t_live_room left join t_user "
            "on t_live_room.owner_id = "
            "t_user.id where t_live_room.id = " +
            std::to_string(room_id));
        if (!ret2.empty()) {
            auto jRet = ret2[0].toJson();
            jRet["cover"] = status.cover;
            jRet["viewer_count"] = status.viewer_count;
            j["live_room"] = jRet;
        }
    }
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "获取成功")
}

void LiveController::enterLive(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "LiveController::enterLive";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "room_id")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    int room_id = params["room_id"].get<int>();
    auto ret1 = redisConn_->get(REDIS_KEY_USER_LIVE_ROOM + std::to_string(userId));
    if (ret1.has_value()) {
        if (std::stoi(ret1.value()) == room_id) {
            RETURN_RESPONSE(HTTP_ERROR_CODE, "您是该直播间的主播，已在直播间内")
        }
    }
    auto ret2 = redisConn_->get(REDIS_KEY_LIVE_ROOM + std::to_string(room_id));
    if (ret2.has_value()) {
        auto status = LiveStatus::fromJson(json::parse(ret2.value()));
        status.viewer_count++;
        redisConn_->set(REDIS_KEY_LIVE_ROOM + std::to_string(room_id), status.toJson().dump());
    }
    redisConn_->sadd(REDIS_KEY_LIVE_ROOM_VIEWER_SET + std::to_string(room_id), std::to_string(userId));
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "进入成功")
}

void LiveController::leaveLive(const HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "LiveController::leaveLive";
    auto params = json::parse(request.body_);
    json j;
    HTTP_CHECK_PARAMS(params, "token", "room_id")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    int room_id = params["room_id"].get<int>();
    auto ret1 = redisConn_->get(REDIS_KEY_LIVE_ROOM + std::to_string(room_id));
    if (ret1.has_value()) {
        auto status = LiveStatus::fromJson(json::parse(ret1.value()));
        status.viewer_count--;
        redisConn_->set(REDIS_KEY_LIVE_ROOM + std::to_string(room_id), status.toJson().dump());
    }
    redisConn_->srem(REDIS_KEY_LIVE_ROOM_VIEWER_SET + std::to_string(room_id), std::to_string(userId));
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "离开成功")
}

void LiveController::handleLiveRoomMsg(const cooper::TcpConnectionPtr& connPtr, const nlohmann::json& params) {
    LOG_DEBUG << "LiveController::handleLiveRoomMsg";
    TCP_CHECK_PARAMS(params, "token", "room_id", "msg")
    std::string token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        return;
    }
    int room_id = params["room_id"].get<int>();
    std::string msg = params["msg"].get<std::string>();
    GET_SQL_CONN_T(sqlConn)
    auto ret1 = sqlConn->query<std::tuple<std::string, std::string>>("select nickname, avatar from t_user where id = " +
                                                                     std::to_string(userId));
    std::vector<std::string> viewerIds;
    redisConn_->smembers(REDIS_KEY_LIVE_ROOM_VIEWER_SET + std::to_string(room_id), std::back_inserter(viewerIds));
    json j;
    j["type"] = PROTOCOL_TYPE_LIVE_ROOM_MSG;
    j["room_id"] = room_id;
    j["user_id"] = userId;
    j["msg"] = msg;
    j["nickname"] = std::get<0>(ret1[0]);
    j["avatar"] = std::get<1>(ret1[0]);
    for (const auto& viewerId : viewerIds) {
        if (std::stoi(viewerId) == userId) {
            continue;
        }
        if (IMStore::getInstance()->isOnlineUser(std::stoi(viewerId))) {
            auto viewerConnPtr = IMStore::getInstance()->getTcpConnection(std::stoi(viewerId));
            viewerConnPtr->sendJson(j);
        }
    }
}

void LiveController::notifyUsersWhenLiveEnd(int roomId) {
    LOG_DEBUG << "LiveController::notifyUsersWhenLiveEnd";
    std::vector<std::string> viewerIds;
    redisConn_->smembers(REDIS_KEY_LIVE_ROOM_VIEWER_SET + std::to_string(roomId), std::back_inserter(viewerIds));
    for (const auto& viewerId : viewerIds) {
        if (IMStore::getInstance()->isOnlineUser(std::stoi(viewerId))) {
            auto connPtr = IMStore::getInstance()->getTcpConnection(std::stoi(viewerId));
            json j;
            j["type"] = PROTOCOL_TYPE_LIVE_ROOM_END;
            j["room_id"] = roomId;
            connPtr->sendJson(j);
        }
    }
}
