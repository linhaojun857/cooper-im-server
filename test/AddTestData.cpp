#include <cooper/util/Logger.hpp>
#include <dbng.hpp>
#include <memory>
#include <mysql.hpp>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"

using namespace cooper;
using namespace ormpp;

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
    "13401792631", "13770093417", "14709743436", "18836736675", "16657284063", "15123016266", "18585581063",
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
    //    for (int i = 2; i <= 20; ++i) {
    //        Friend f;
    //        f.id = 0;
    //        f.a_id = 1;
    //        f.b_id = i;
    //        f.group_type = 0;
    //        sqlConn->insert(f);
    //    }
    for (int i = 2; i <= 20; ++i) {
        Friend f;
        f.id = 0;
        f.a_id = 2;
        if (i == 2) {
            f.b_id = 1;
        } else {
            f.b_id = i;
        }
        f.group_type = 0;
        sqlConn->insert(f);
    }
}

int main() {
    std::shared_ptr<dbng<mysql>> sqlConn = std::make_shared<dbng<mysql>>();
    if (!sqlConn->connect(MYSQL_SERVER_IP, MYSQL_SERVER_USERNAME, MYSQL_SERVER_PASSWORD, MYSQL_SERVER_DATABASE)) {
        LOG_ERROR << "connect mysql failed";
        return -1;
    }
    if (!sqlConn->create_datatable<User>(ormpp_auto_key{"id"}) ||
        !sqlConn->create_datatable<Friend>(ormpp_auto_key{"id"})) {
        LOG_ERROR << "create table user failed";
        return -1;
    }
    auto friends = sqlConn->query<User>(
        "select user.*\n"
        "from (select * from friend where a_id = 1) as t1\n"
        "left join user on t1.b_id = user.id;");
    std::cout << friends.size() << std::endl;
    return 0;
}
