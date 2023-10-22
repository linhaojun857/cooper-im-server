#ifndef define_IMDEFINE_HPP_
#define define_IMDEFINE_HPP_

#define MYSQL_SERVER_IP ("172.18.48.1")
#define MYSQL_SERVER_PORT (3306)
#define MYSQL_SERVER_USERNAME ("root")
#define MYSQL_SERVER_PASSWORD ("20030802")
#define MYSQL_SERVER_DATABASE ("cooper_im")

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
    j_err["msg"] = "缺少token";              \
    connPtr->sendJson(j_err);                \
    return;

#define PROTOCOL_TYPE_BASE (1000)
#define PROTOCOL_TYPE_AUTH_MSG (PROTOCOL_TYPE_BASE + 1)
#define PROTOCOL_TYPE_ERROR_MSG (PROTOCOL_TYPE_BASE + 2)
#define PROTOCOL_TYPE_FRIEND_APPLY_NOTIFY_I (PROTOCOL_TYPE_BASE + 3)
#define PROTOCOL_TYPE_FRIEND_APPLY_NOTIFY_P (PROTOCOL_TYPE_BASE + 4)
#define PROTOCOL_TYPE_FRIEND_ENTITY (PROTOCOL_TYPE_BASE + 5)

#endif
