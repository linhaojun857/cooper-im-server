#include "JwtUtil.hpp"

std::string JwtUtil::createToken(int id) {
    auto token = jwt::create()
                     .set_issuer("cooper")
                     .set_type("JWT")
                     .set_payload_claim("id", jwt::claim(std::to_string(id)))
                     .sign(jwt::algorithm::hs256{"secret"});
    return token;
}

int JwtUtil::parseToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto payload = decoded.get_payload_claim("id");
        return std::stoi(payload.as_string());
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return -1;
}
