#include <cooper/util/Date.hpp>
#include <cooper/util/Logger.hpp>
#include <dbng.hpp>
#include <memory>
#include <mysql.hpp>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"

using namespace cooper;
using namespace ormpp;

struct Model {
    int id{};
    std::string name;
    time_t timestamp{};
};

std::ostream& operator<<(std::ostream& os, const Model& m) {
    os << "id: " << m.id << ", name: " << m.name << ", timestamp: " << m.timestamp;
    return os;
}

REFLECTION(Model, id, name, timestamp)

struct Test {
    int id{};
    std::string username;
    std::string password;
};

REFLECTION(Test, id, username, password)

int main() {
    std::shared_ptr<dbng<mysql>> sqlConn = std::make_shared<dbng<mysql>>();
    if (!sqlConn->connect(MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME, MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE)) {
        LOG_ERROR << "connect mysql failed";
        return -1;
    }
    if (!sqlConn->create_datatable<Model>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Test>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table user failed";
        return -1;
    }
    {
        // ???
        auto ret = sqlConn->query<std::tuple<std::string>>("select username from user where id > ?", 10);
        std::cout << ret.size() << std::endl;
    }
    //    {
    //        for (int i = 0; i < 10; ++i) {
    //            Test test;
    //            test.id = 0;
    //            test.username = "username" + std::to_string(i);
    //            test.password = "password" + std::to_string(i);
    //            sqlConn->insert(test);
    //        }
    //    }
    {
        // 测试sql注入
        auto ret1 = sqlConn->query<Test>("username = '" + std::string("username1") + "'");
        std::cout << "before sql injection: " << ret1.size() << std::endl;
        auto ret2 = sqlConn->query<Test>("username = '" + std::string("username1") + "' or 1=1");
        std::cout << "after sql injection: " << ret2.size() << std::endl;
    }
    return 0;
}
