#ifndef controller_UserController_hpp
#define controller_UserController_hpp
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;

class UserController {
public:
    explicit UserController(std::shared_ptr<dbng<mysql>> mysql);

    void getVfCode(const HttpRequest& request, HttpResponse& response);

    void userLogin(const HttpRequest& request, HttpResponse& response);

    void userRegister(const HttpRequest& request, HttpResponse& response);

    void getSyncState(const HttpRequest& request, HttpResponse& response);

    void getFriends(const HttpRequest& request, HttpResponse& response);

    void search(const HttpRequest& request, HttpResponse& response);

    void addFriend(const HttpRequest& request, HttpResponse& response);

    void responseFriendApply(const HttpRequest& request, HttpResponse& response);

    void handleAuthMsg(const TcpConnectionPtr& connPtr, const json& params);

private:
    std::shared_ptr<dbng<mysql>> sqlConn_;
};

#endif
