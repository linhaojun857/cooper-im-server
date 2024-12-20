#include <sw/redis++/redis++.h>

#include <cooper/util/Logger.hpp>
#include <dbng.hpp>
#include <memory>
#include <mysql.hpp>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"

using namespace cooper;
using namespace ormpp;
using namespace sw::redis;

std::vector<std::string> testAvatars = {
    "http://thirdqq.qlogo.cn/g?b=oidb&k=jWGicCEAEOGTsXtzplbsBSA&s=40&t=1658928554",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=OgzJGGDUAQWW996sYuIblg&s=40&t=1648976853",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=XkKCoFEue3N8F66EgTRrzQ&s=40&t=1659006735",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=MIgSsjicQfRj7cxIWtd9iaVQ&s=40&t=1657901905",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=msjpp0UlW7UeF2wdODjmZg&s=40&t=1633943518",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=nFHptgpec2khZffJ6tYXHA&s=40&t=1659875078",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=MZGdfr2siaeicUgib1JbicZWbQ&s=40&t=1653931352",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=pia1Uwu0nGicHQYS8rkc4fYQ&s=40&t=1555542524",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=OqTaCibiclZiaV0WjlBs74YVw&s=40&t=1571802774",
    "http://thirdqq.qlogo.cn/g?b=oidb&k=ne4WYYMKm1DkPBzTU6vt6A&s=40&t=1605537409",
    "https://static.linhaojun.top/aurora/avatar/01e049223b03fc230c2b28ae16e9280d.jpg",
    "https://static.linhaojun.top/aurora/avatar/0f4cd4fdabdbd86f1d654fb49a90fafb.jpg",
    "https://static.linhaojun.top/aurora/avatar/281fe50894fdfa0f86f7f9690b526097.png",
    "https://static.linhaojun.top/aurora/avatar/84bfed455789c221619bfe58b0ad2b85.jpeg",
    "https://static.linhaojun.top/aurora/avatar/d7f20623c806e6739cdf030df3610113.jpg",
    "https://static.linhaojun.top/aurora/avatar/c17eca6537e93a2a3f3afc9ebe293589.jpg",
    "https://static.linhaojun.top/aurora/avatar/e608f63258d1a50ab0da6965649c2a37.jpg",
    "https://static.linhaojun.top/aurora/avatar/541e55aaa3752cdcd00367d97ae895ce.jpg",
    "https://static.linhaojun.top/aurora/avatar/f603d1053fb3d40f84e759c27c347a28.jpeg",
    "https://static.linhaojun.top/aurora/avatar/8ac9c63f9d0ba11c7d4d45191c406d18.jpg",
};

std::vector<std::string> testUsernames = {
    "13401792631", "13486186186", "14709743436", "18836736675", "16657284063", "15123016266", "18585581063",
    "17194534287", "17392507674", "15273389032", "13923138574", "14618295534", "18885211064", "15136798595",
    "14104066626", "14546871073", "17248648783", "15680537493", "18233604867", "18883629511", "13346438448"};

std::vector<std::string> testNicknames = {"开心的火龙果", "二爷",      "你仔细听", "小乖乖",         "忆生i",
                                          "杨酷酷",       "五行缺钱",  "MRBEE",    "星川",           "怀念",
                                          "没毛的小狐狸", "八尺妖剑",  "提露",     "一个超人的角色", "所念皆星河",
                                          "远辰",         "Cold moon", "永恒",     "ZVerify",        "明天一定吃早饭"};

std::vector<std::string> testStatuses = {"忙碌", "在线", "离开", "求锦鲤", "发呆", "胡思乱想"};

std::vector<std::string> testFeelings = {"每一天都是一个新的开始。", "梦想，永不放弃。",
                                         "阳光总在风雨后。",         "不忘初心，方得始终。",
                                         "做自己，因为你独一无二。", "生活不止眼前的苟且，还有诗和远方。",
                                         "幸福是一种心态。",         "努力不一定成功，但不努力必定失败。",
                                         "心若向阳，无谓阴霾。",     "走自己的路，让别人说去吧。",
                                         "生活需要笑容和感激。",     "希望是前进的动力。",
                                         "在黑夜中寻找光明。",       "活在当下，享受每一刻。",
                                         "坚持就是胜利。",           "不怕失败，只怕不再尝试。",
                                         "相信自己，你能做到。",     "勇敢前行，未来在等待。",
                                         "不忙于奔波，不迷失方向。", "活出自己的精彩。"};

std::vector<std::string> testGroupNames = {"Aurora博客交流群", "SpringBoot交流群", "Java交流群",  "MySQL交流群",
                                           "Linux交流群",      "Redis交流群",      "ES甲流群",    "MyBatis交流群",
                                           "Kafka交流群",      "RabbitMQ交流群",   "测试交流群1", "测试交流群2",
                                           "测试交流群3",      "测试交流群4",      "测试交流群5", "测试交流群6",
                                           "测试交流群7",      "测试交流群8",      "测试交流群9", "测试交流群10"};

void addUserTestData(const std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 0; i < 20; ++i) {
        User user;
        user.id = 0;
        user.username = testUsernames[i % testUsernames.size()];
        user.nickname = testNicknames[i % testNicknames.size()];
        user.password = "123";
        user.avatar = testAvatars[i % testAvatars.size()];
        user.status = testStatuses[i % testStatuses.size()];
        user.feeling = testFeelings[i % testFeelings.size()];
        sqlConn->insert<User>(user);
    }
}

void addFriendTestData(const std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 2; i <= 12; ++i) {
        std::string session_id = IMUtil::generateUUid();
        Friend f{};
        f.id = 0;
        f.a_id = 1;
        f.b_id = i;
        f.session_id = session_id;
        f.group_type = 0;
        sqlConn->insert(f);
        f.a_id = i;
        f.b_id = 1;
        f.group_type = 0;
        sqlConn->insert(f);
    }
}

void addPersonMessageTestData(const std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 0; i < 50; ++i) {
        PersonMessage pm;
        auto ret = sqlConn->query<std::tuple<std::string>>(
            "select session_id from t_friend where a_id = ? and b_id = ?", 1, 3);
        pm.session_id = std::get<0>(ret[0]);
        pm.id = 0;
        if (i % 2 == 0) {
            pm.from_id = 1;
            pm.to_id = 3;
        } else {
            pm.from_id = 3;
            pm.to_id = 1;
        }
        pm.message_type = 0;
        pm.message = "这是测试消息B-" + std::to_string(i);
        pm.timestamp = time(nullptr);
        sqlConn->insert(pm);
    }
}

void addGroupTestData(const std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 0; i < 20; ++i) {
        Group group;
        group.id = 0;
        group.session_id = IMUtil::generateUUid();
        group.group_num = IMUtil::generateGroupNum();
        group.name = testGroupNames[i % testGroupNames.size()];
        group.avatar = DEFAULT_GROUP_AVATAR;
        if (i < 10) {
            group.owner_id = 1;
        } else {
            group.owner_id = 2;
        }
        group.description = "这是测试群组-" + std::to_string(i);
        sqlConn->insert(group);
        auto id = sqlConn->query<std::tuple<int>>("select LAST_INSERT_ID()");
        UserGroup userGroup(group.owner_id, std::get<0>(id[0]));
        sqlConn->insert(userGroup);
    }
}

void addUserGroupTestData(std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 2; i <= 10; ++i) {
        UserGroup ug(i, 1);
        sqlConn->insert(ug);
    }
}

void AddGroupMessageTestData(std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 1; i <= 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            std::string message = "群组测试消息" + std::to_string(i) + "-" + std::to_string(j);
            GroupMessage gm;
            gm.id = 0;
            gm.from_id = i;
            gm.group_id = 1;
            gm.message_type = 0;
            gm.message = message;
            gm.timestamp = time(nullptr);
            sqlConn->insert(gm);
        }
    }
}

void AddPyqTestData(std::shared_ptr<dbng<mysql>>& sqlConn) {
    for (int i = 1; i <= 20; ++i) {
        Pyq pyq;
        pyq.id = 0;
        pyq.user_id = 1;
        pyq.content = "这是朋友圈测试消息" + std::to_string(i);
        std::vector<std::string> imageUrls;
        imageUrls.emplace_back("http://localhost:9999/static/upload/1/3cf76842b29e43e0831581ec3c309917.jpg");
        imageUrls.emplace_back("http://localhost:9999/static/upload/1/41acb7102a864437a83c57c54e796463.jpg");
        imageUrls.emplace_back("http://localhost:9999/static/upload/1/b51728df438744e394442f5de16aee9b.jpg");
        json j = imageUrls;
        pyq.image_urls = j.dump();
        pyq.timestamp = time(nullptr) - (20 - i) * 100000;
        sqlConn->insert(pyq);
    }
}

int main() {
    std::shared_ptr<dbng<mysql>> sqlConn = std::make_shared<dbng<mysql>>();
    if (!sqlConn->connect(MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME, MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE)) {
        LOG_ERROR << "connect mysql failed";
        return -1;
    }
    if (!sqlConn->create_datatable<User>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Friend>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Notify>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<FriendApply>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<PersonMessage>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Group>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<UserGroup>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<GroupApply>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<GroupMessage>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Pyq>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<PyqComment>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table failed";
        return -1;
    }
    ConnectionOptions connectionOptions;
    connectionOptions.host = REDIS_SERVER_IP;
    connectionOptions.port = REDIS_SERVER_PORT;
    connectionOptions.password = REDIS_SERVER_PASSWORD;
    connectionOptions.db = REDIS_SERVER_DATABASE;
    std::shared_ptr<Redis> redisConn = std::make_shared<Redis>(connectionOptions);
    IMStore::getInstance()->setRedisConn(redisConn);

    AddPyqTestData(sqlConn);
    return 0;
}
