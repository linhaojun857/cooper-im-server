#include "FileController.hpp"

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "entity/Entity.hpp"
#include "store/IMStore.hpp"

FileController::FileController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
}

void FileController::upload(const cooper::HttpRequest& request, cooper::HttpResponse& response) {

}
