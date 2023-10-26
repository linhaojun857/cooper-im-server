#include <nlohmann/json.hpp>
#include <iostream>

using namespace nlohmann;

int main() {
    std::vector<int> v = {1, 2, 3};
    json j = v;
    std::cout << j << std::endl;
}
