#ifndef service_UserService_HPP_
#define service_UserService_HPP_
#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;

class UserService {
public:
    explicit UserService(std::shared_ptr<dbng<mysql>> mysql);

    void getVfCode(const HttpRequest& request, HttpResponse& response);

    void userLogin(const HttpRequest& request, HttpResponse& response);

    void userRegister(const HttpRequest& request, HttpResponse& response);

private:
    std::shared_ptr<dbng<mysql>> sqlConn_;
};

#endif
