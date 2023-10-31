#include "IMUtil.hpp"

#include <uuid/uuid.h>

std::string IMUtil::generateUUid() {
    uuid_t uuid;
    uuid_generate(uuid);
    char str[37];
    uuid_unparse(uuid, str);
    return {str};
}
