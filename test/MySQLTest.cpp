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

int main() {
    std::shared_ptr<dbng<mysql>> sqlConn = std::make_shared<dbng<mysql>>();
    if (!sqlConn->connect(MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME, MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE)) {
        LOG_ERROR << "connect mysql failed";
        return -1;
    }
    if (!sqlConn->create_datatable<Model>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table user failed";
        return -1;
    }
    auto pms = sqlConn->query<PersonMessage>("from_id = 1 or to_id = 1");
    std::cout << pms.size() << std::endl;
    return 0;
}
