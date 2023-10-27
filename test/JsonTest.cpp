#include <iostream>
#include <nlohmann/json.hpp>

using namespace nlohmann;

int main() {
    //    std::vector<int> v = {1, 2, 3};
    //    json j = v;
    //    std::cout << j << std::endl;
    std::vector<std::pair<int, int>> v;
    //    v.emplace_back(1, 2);
    //    v.emplace_back(3, 4);
    //    v.emplace_back(5, 6);
    //    json j = v;
    //    std::cout << j.dump() << std::endl;
    v = json::parse("[[1,2],[3,4],[5,6]]");
    for (const auto& p : v) {
        std::cout << p.first << " " << p.second << std::endl;
    }
}
