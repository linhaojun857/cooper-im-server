#ifndef define_IMDEFINE_HPP_
#define define_IMDEFINE_HPP_

#define MYSQL_SERVER_IP ("172.18.48.1")
#define MYSQL_SERVER_PORT (3306)
#define MYSQL_SERVER_USERNAME ("root")
#define MYSQL_SERVER_PASSWORD ("20030802")
#define MYSQL_SERVER_DATABASE ("cooper_im")
#define MYSQL_SERVER_TIMEOUT (100)
#define MYSQL_CONNECTION_POOL_SIZE (10)

#define REDIS_SERVER_IP ("127.0.0.1")
#define REDIS_SERVER_PORT (6379)
#define REDIS_SERVER_PASSWORD ("123456")
#define REDIS_SERVER_DATABASE (0)
#define REDIS_CONNECTION_POOL_SIZE (3)

#define HTTP_SUCCESS_CODE (20000)
#define HTTP_ERROR_CODE (30000)

#define DEFAULT_USER_AVATAR ("https://static.linhaojun.top/aurora/config/0af1901da1e64dfb99bb61db21e716c4.jpeg")
#define DEFAULT_USER_FEELING ("这个人很懒，什么也没留下")
#define DEFAULT_USER_STATUS ("在线")

#define RETURN_RESPONSE(code, msg) \
    j["code"] = code;              \
    j["msg"] = msg;                \
    response.body_ = j.dump();     \
    return;

#define RETURN_ERROR(msg_in)                 \
    json j_err;                              \
    j_err["type"] = PROTOCOL_TYPE_ERROR_MSG; \
    j_err["msg"] = msg_in;                   \
    connPtr->sendJson(j_err);                \
    return;

#define TCP_CHECK_PARAMS(params, ...)        \
    {                                        \
        std::string param[] = {__VA_ARGS__}; \
        for (auto& i : param) {              \
            if (!params.contains(i)) {       \
                RETURN_ERROR("缺少" + i)     \
            }                                \
        }                                    \
    }

#define HTTP_CHECK_PARAMS(params, ...)                   \
    std::string param[] = {__VA_ARGS__};                 \
    for (auto& i : param) {                              \
        if (!params.contains(i)) {                       \
            RETURN_RESPONSE(HTTP_ERROR_CODE, "缺少" + i) \
        }                                                \
    }

#define GET_SQL_CONN(sqlConn, sqlConnPool)        \
    auto sqlConn = sqlConnPool->get();            \
    conn_guard guard(sqlConn);                    \
    if (!sqlConn) {                               \
        LOG_ERROR << "get sql connection failed"; \
    }

#define GET_SQL_CONN_H(sqlConn)                        \
    auto sqlConn = sqlConnPool_->get();                \
    conn_guard guard(sqlConn);                         \
    if (!sqlConn) {                                    \
        LOG_ERROR << "get sql connection failed";      \
        RETURN_RESPONSE(HTTP_ERROR_CODE, "服务端异常") \
    }

#define GET_SQL_CONN_T(sqlConn)                   \
    auto sqlConn = sqlConnPool_->get();           \
    conn_guard guard(sqlConn);                    \
    if (!sqlConn) {                               \
        LOG_ERROR << "get sql connection failed"; \
        RETURN_ERROR("服务端异常")                \
    }

#define PROTOCOL_TYPE_BASE (1000)
#define PROTOCOL_TYPE_BUSINESS_AUTH_MSG (PROTOCOL_TYPE_BASE + 1)
#define PROTOCOL_TYPE_ERROR_MSG (PROTOCOL_TYPE_BASE + 2)
#define PROTOCOL_TYPE_FRIEND_APPLY_NOTIFY_I (PROTOCOL_TYPE_BASE + 3)
#define PROTOCOL_TYPE_FRIEND_APPLY_NOTIFY_P (PROTOCOL_TYPE_BASE + 4)
#define PROTOCOL_TYPE_FRIEND_ENTITY (PROTOCOL_TYPE_BASE + 5)
#define PROTOCOL_TYPE_SYNC_COMPLETE_MESSAGE (PROTOCOL_TYPE_BASE + 6)
#define PROTOCOL_TYPE_PERSON_MESSAGE_SEND (PROTOCOL_TYPE_BASE + 7)
#define PROTOCOL_TYPE_PERSON_MESSAGE_RECV (PROTOCOL_TYPE_BASE + 8)
#define PROTOCOL_TYPE_GROUP_APPLY_NOTIFY_I (PROTOCOL_TYPE_BASE + 9)
#define PROTOCOL_TYPE_GROUP_APPLY_NOTIFY_P (PROTOCOL_TYPE_BASE + 10)
#define PROTOCOL_TYPE_GROUP_ENTITY (PROTOCOL_TYPE_BASE + 11)
#define PROTOCOL_TYPE_GROUP_MESSAGE_SEND (PROTOCOL_TYPE_BASE + 12)
#define PROTOCOL_TYPE_GROUP_MESSAGE_RECV (PROTOCOL_TYPE_BASE + 13)
#define PROTOCOL_TYPE_LIVE_ROOM_END (PROTOCOL_TYPE_BASE + 14)
#define PROTOCOL_TYPE_LIVE_ROOM_MSG_SEND (PROTOCOL_TYPE_BASE + 15)
#define PROTOCOL_TYPE_LIVE_ROOM_MSG_RECV (PROTOCOL_TYPE_BASE + 16)
#define PROTOCOL_TYPE_LIVE_ROOM_UPDATE_VIEWER_COUNT (PROTOCOL_TYPE_BASE + 17)
#define PROTOCOL_TYPE_VIDEO_CALL_REQUEST (PROTOCOL_TYPE_BASE + 18)
#define PROTOCOL_TYPE_VIDEO_CALL_RESPONSE (PROTOCOL_TYPE_BASE + 19)
#define PROTOCOL_TYPE_VIDEO_CALL_END (PROTOCOL_TYPE_BASE + 20)
#define PROTOCOL_TYPE_AUDIO_CALL_REQUEST (PROTOCOL_TYPE_BASE + 21)
#define PROTOCOL_TYPE_AUDIO_CALL_RESPONSE (PROTOCOL_TYPE_BASE + 22)
#define PROTOCOL_TYPE_AUDIO_CALL_END (PROTOCOL_TYPE_BASE + 23)

#define PROTOCOL_TYPE_MEDIA_AUTH_MSG (PROTOCOL_TYPE_BASE + 101)
#define PROTOCOL_TYPE_VIDEO_CALL_AUDIO_FRAME (PROTOCOL_TYPE_BASE + 102)
#define PROTOCOL_TYPE_VIDEO_CALL_VIDEO_FRAME (PROTOCOL_TYPE_BASE + 103)
#define PROTOCOL_TYPE_AUDIO_CALL_AUDIO_FRAME (PROTOCOL_TYPE_BASE + 104)

#define REDIS_KEY_ONLINE_USERS "online_users"
#define REDIS_KEY_NOTIFY_QUEUE_PREFIX "notify_queue:"
#define REDIS_KEY_PERSON_OFFLINE_MSG "person_offline_msg:"
#define REDIS_KEY_GROUP_OFFLINE_MSG "group_offline_msg:"
#define REDIS_KEY_SYNC_STATE_PREFIX "sync_state:"
#define REDIS_KEY_SYNC_FRIEND_ENTITY_PREFIX "sync_friend_entity:"
#define REDIS_KEY_CUR_YEAR_GROUP_NUM "cur_year_group_num"
#define REDIS_KEY_SHARD_UPLOAD_FILE "shard_upload_file:"
#define REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX "shard_upload_file_mutex:"
#define REDIS_KEY_LIVE_ROOM "live_room:"
#define REDIS_KEY_LIVE_ROOM_SET "live_room_set"
#define REDIS_KEY_USER_LIVE_ROOM "user_live_room:"
#define REDIS_KEY_LIVE_ROOM_VIEWER_SET "live_room_viewer_set:"
#define REDIS_KEY_LIVE_ROOM_MSG "live_room_msg:"
#define REDIS_KEY_VIDEO_CALL "video_call:"

#define SYNC_DATA_FRIEND_ENTITY_INSERT 1
#define SYNC_DATA_FRIEND_ENTITY_UPDATE 2
#define SYNC_DATA_FRIEND_ENTITY_DELETE 3

#define SYNC_DATA_PERSON_MESSAGE_INSERT 1
#define SYNC_DATA_PERSON_MESSAGE_DELETE 2
#define SYNC_DATA_GROUP_MESSAGE_INSERT 1
#define SYNC_DATA_GROUP_MESSAGE_DELETE 2

#define MSG_TYPE_TEXT 0
#define MSG_TYPE_FILE 1

#define DEFAULT_GROUP_AVATAR "http://localhost:9999/static/img/group.png"
#define UPLOAD_PATH "/home/linhaojun/cpp-code/cooper-im-server/static/upload/"
#define FILE_URL_PREFIX "http://localhost:9999/static/upload/"

#endif
