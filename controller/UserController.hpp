#ifndef service_UserService_HPP_
#define service_UserService_HPP_
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

    void search(const HttpRequest& request, HttpResponse& response);

    void addFriend(const HttpRequest& request, HttpResponse& response);

    void handleAuthMsg(const TcpConnectionPtr& connPtr, const json& params);

private:
    std::shared_ptr<dbng<mysql>> sqlConn_;
};

#endif
