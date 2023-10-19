#include "util/JwtUtil.hpp"

int main() {
    int id_in = 109;
    auto token = JwtUtil::createToken(id_in);
    std::cout << token << std::endl;
    int id_out = JwtUtil::parseToken(token);
    std::cout << id_out << std::endl;
}
