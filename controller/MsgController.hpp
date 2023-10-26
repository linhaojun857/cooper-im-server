#ifndef controller_MsgController_hpp
#define controller_MsgController_hpp

#include <cooper/net/Http.hpp>
#include <dbng.hpp>
#include <mysql.hpp>

using namespace cooper;
using namespace ormpp;

class MsgController {
public:
    explicit MsgController(std::shared_ptr<dbng<mysql>> mysql);

    void handlePersonSendMsg(const TcpConnectionPtr& connPtr, const json& params);

private:
    std::shared_ptr<dbng<mysql>> sqlConn_;
};

#endif
