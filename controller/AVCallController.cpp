#include "AVCallController.hpp"

#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/JwtUtil.hpp"

AVCallController::AVCallController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
    IMStore::getInstance()->registerAVCallController(this);
}

void AVCallController::handleVideoCallRequest(const TcpConnectionPtr& connPtr, const json& params) {
    LOG_DEBUG << "AVCallController::handleVideoCallRequest";
    TCP_CHECK_PARAMS(params, "token", "from_id", "to_id");
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("token无效")
    }
    auto from_id = params["from_id"].get<int>();
    auto to_id = params["to_id"].get<int>();
    if (!IMStore::getInstance()->haveBusinessTcpConnection(to_id)) {
        json j;
        j["type"] = PROTOCOL_TYPE_VIDEO_CALL_RESPONSE;
        j["code"] = 1;
        connPtr->sendJson(j);
        return;
    }
    redisConn_->set(REDIS_KEY_VIDEO_CALL + std::to_string(from_id), std::to_string(to_id));
    redisConn_->set(REDIS_KEY_VIDEO_CALL + std::to_string(to_id), std::to_string(from_id));
    auto toConnPtr = IMStore::getInstance()->getBusinessTcpConnection(to_id);
    json j;
    j["type"] = PROTOCOL_TYPE_VIDEO_CALL_REQUEST;
    j["from_id"] = from_id;
    j["to_id"] = to_id;
    toConnPtr->sendJson(j);
}

void AVCallController::handleVideoCallResponse(const TcpConnectionPtr& connPtr, const json& params) {
    LOG_DEBUG << "AVCallController::handleVideoCallResponse";
    TCP_CHECK_PARAMS(params, "token", "from_id", "to_id");
    auto token = params["token"].get<std::string>();
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_ERROR("token无效")
    }
    auto from_id = params["from_id"].get<int>();
    auto to_id = params["to_id"].get<int>();
    if (!IMStore::getInstance()->haveBusinessTcpConnection(to_id)) {
        return;
    }
    auto toConnPtr = IMStore::getInstance()->getBusinessTcpConnection(to_id);
    json j;
    j["type"] = PROTOCOL_TYPE_VIDEO_CALL_RESPONSE;
    j["from_id"] = from_id;
    j["to_id"] = to_id;
    j["code"] = params["code"].get<int>();
    toConnPtr->sendJson(j);
}

void AVCallController::handleMediaAuthMsg(const cooper::TcpConnectionPtr& connPtr, const char* buf, size_t len) {
    LOG_DEBUG << "AVCallController::handleMediaAuthMsg";
    std::string token = std::string(buf + sizeof(int), len - sizeof(int));
    int userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        return;
    }
    IMStore::getInstance()->addMediaTcpConnection(userId, connPtr);
}

void AVCallController::handleVideoCallAudioFrame(const TcpConnectionPtr& connPtr, const char* buf, size_t len) {
    int to_id = *(int*)(buf + sizeof(int) * 2);
    if (!IMStore::getInstance()->haveMediaTcpConnection(to_id)) {
        return;
    }
    auto toConnPtr = IMStore::getInstance()->getMediaTcpConnection(to_id);
    toConnPtr->send(&len, sizeof(int));
    toConnPtr->send(buf, len);
}

void AVCallController::handleVideoCallVideoFrame(const TcpConnectionPtr& connPtr, const char* buf, size_t len) {
    int to_id = *(int*)(buf + sizeof(int) * 2);
    if (!IMStore::getInstance()->haveMediaTcpConnection(to_id)) {
        return;
    }
    auto toConnPtr = IMStore::getInstance()->getMediaTcpConnection(to_id);
    toConnPtr->send(&len, sizeof(int));
    toConnPtr->send(buf, len);
}
