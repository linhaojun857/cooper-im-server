#include "FileController.hpp"

#include <sys/stat.h>

#include <cooper/util/Logger.hpp>
#include <regex>
#include <utility>

#include "define/IMDefine.hpp"
#include "entity/Entity.hpp"
#include "store/IMStore.hpp"
#include "util/IMUtil.hpp"
#include "util/JwtUtil.hpp"
#include "util/MD5.hpp"

FileController::FileController(connection_pool<dbng<mysql>>* sqlConnPool, std::shared_ptr<Redis> redisConn)
    : sqlConnPool_(sqlConnPool), redisConn_(std::move(redisConn)) {
}

void FileController::checkBeforeUpload(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FileController::checkBeforeUpload";
    json j;
    auto params = json::parse(request.body_);
    HTTP_CHECK_PARAMS(params, "token", "filename", "file_md5")
    auto token = params["token"].get<std::string>();
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    auto filename = params["filename"].get<std::string>();
    auto file_md5 = params["file_md5"].get<std::string>();
    GET_SQL_CONN_H(sqlConn)
    auto ret = sqlConn->query<File>("select * from t_file where file_md5 = '" + file_md5 +
                                    "' and user_id = " + std::to_string(userId));
    if (ret.empty()) {
        j["exist"] = 0;
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "文件不存在")
    }
    ret[0].file_name = filename;
    sqlConn->update(ret[0]);
    j["exist"] = 1;
    j["file_url"] = ret[0].file_url;
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "文件已存在")
}

void FileController::upload(const cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FileController::upload";
    json j;
    auto token = request.getHeaderValue("token");
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    const auto& fileData = request.getMultiPartFormData("file");
    auto file_md5 = request.getMultiPartFormData("file_md5").content;
    auto ret = sqlConn->query<File>("select * from t_file where file_md5 = '" + file_md5 +
                                    "' and user_id = " + std::to_string(userId));
    if (!ret.empty()) {
        ret[0].file_name = fileData.filename;
        sqlConn->update(ret[0]);
        j["file_url"] = ret[0].file_url;
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "上传成功")
    }
    std::string fileType = IMUtil::getFileType(fileData.filename);
    std::string randomFileName = IMUtil::generateRandomFileName(fileType);
    std::string fileUrl = FILE_URL_PREFIX + std::to_string(userId) + "/" + randomFileName;
    std::string filePath = UPLOAD_PATH + std::to_string(userId) + "/" + randomFileName;
    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "上传失败")
    }
    write(fd, fileData.content.data(), fileData.content.size());
    fsync(fd);
    lseek(fd, 0, SEEK_SET);
    struct stat statBuf {};
    size_t fileSize;
    if (::stat(filePath.c_str(), &statBuf) == 0) {
        fileSize = statBuf.st_size;
    } else {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    std::string content(fileSize, '\0');
    read(fd, content.data(), fileSize);
    close(fd);
    std::string md5 = MD5(content).hexdigest();
    if (md5 != file_md5) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    File file(fileType, fileData.filename, fileUrl, static_cast<long>(fileData.content.size()), md5, userId,
              time(nullptr));
    sqlConn->insert(file);
    j["file_url"] = fileUrl;
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "上传成功")
}

void FileController::shardUpload(const HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "FileController::shardUpload";
    json j;
    auto token = request.getHeaderValue("token");
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    const auto& fileData = request.getMultiPartFormData("file");
    auto filename = fileData.filename;
    auto su_id = request.getMultiPartFormData("su_id").content;
    auto shard_index = std::stoi(request.getMultiPartFormData("shard_index").content);
    auto shard_size = std::stoi(request.getMultiPartFormData("shard_size").content);
    auto shard_count = std::stoi(request.getMultiPartFormData("shard_count").content);
    auto file_md5 = request.getMultiPartFormData("file_md5").content;
    GET_SQL_CONN_H(sqlConn)
    auto ret = sqlConn->query<File>("select * from t_file where file_md5 = '" + file_md5 +
                                    "' and user_id = " + std::to_string(userId));
    if (!ret.empty()) {
        if (shard_index == 1) {
            ret[0].file_name = fileData.filename;
            sqlConn->update(ret[0]);
        }
        j["complete"] = 1;
        j["file_url"] = ret[0].file_url;
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "上传成功")
    }
    std::string fileType = IMUtil::getFileType(filename);
    std::string randomFileName = su_id + "." + fileType;
    std::string filePath = UPLOAD_PATH + std::to_string(userId) + "/" + randomFileName;
    int fd1 = open(filePath.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd1 < 0) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "上传失败")
    }
    lseek(fd1, shard_index * shard_size, SEEK_SET);
    write(fd1, fileData.content.data(), fileData.content.size());
    close(fd1);
    try {
        while (!redisConn_->setnx(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id, "1")) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        auto ret2 = redisConn_->get(REDIS_KEY_SHARD_UPLOAD_FILE + su_id);
        ShardUploadStatus status;
        if (!ret2.has_value()) {
            status.count = 1;
            status.shardIds.insert(shard_index);
            if (status.count != shard_count) {
                redisConn_->set(REDIS_KEY_SHARD_UPLOAD_FILE + su_id, status.toJson().dump());
                redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
                j["complete"] = 0;
                RETURN_RESPONSE(HTTP_SUCCESS_CODE, "该分片上传成功")
            }
        } else {
            status = ShardUploadStatus::fromJson(json::parse(ret2.value()));
            ++status.count;
            status.shardIds.insert(shard_index);
            redisConn_->set(REDIS_KEY_SHARD_UPLOAD_FILE + su_id, status.toJson().dump());
            if (status.count != shard_count) {
                redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
                j["complete"] = 0;
                RETURN_RESPONSE(HTTP_SUCCESS_CODE, "该分片上传成功")
            }
        }
        struct stat statBuf {};
        size_t fileSize;
        if (::stat(filePath.c_str(), &statBuf) == 0) {
            fileSize = statBuf.st_size;
        } else {
            redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
            RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
        }
        int fd2 = open(filePath.c_str(), O_RDONLY, 0644);
        if (fd2 < 0) {
            redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
            RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
        }
        std::string content(fileSize, '\0');
        read(fd2, content.data(), fileSize);
        close(fd2);
        std::string md5 = MD5(content).hexdigest();
        if (md5 != file_md5) {
            redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
            RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
        }
        std::string fileUrl = FILE_URL_PREFIX + std::to_string(userId) + "/" + randomFileName;
        File file(fileType, filename, fileUrl, static_cast<long>(fileData.content.size()), md5, userId, time(nullptr));
        sqlConn->insert(file);
        redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE + su_id);
        redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
        j["complete"] = 1;
        j["file_url"] = fileUrl;
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "文件上传成功")
    } catch (const std::exception& e) {
        LOG_ERROR << e.what();
        redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
        RETURN_RESPONSE(HTTP_ERROR_CODE, "上传失败")
    }
}
