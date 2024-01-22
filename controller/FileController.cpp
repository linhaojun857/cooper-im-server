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
    IMStore::getInstance()->registerFileController(this);
}

void FileController::checkBeforeUpload(cooper::HttpRequest& request, cooper::HttpResponse& response) {
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

void FileController::upload(cooper::HttpRequest& request, cooper::HttpResponse& response) {
    LOG_DEBUG << "FileController::upload";
    json j;
    auto token = request.getHeaderValue("token");
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    std::string file_md5;
    std::string filename;
    std::string fileType;
    std::string fileUrl;
    std::string filePath;
    bool isExist = false;
    int fd = -1;
    MultiPartWriteCallbackMap writeCallbacks;
    writeCallbacks["file_md5"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_CONTENT) {
            file_md5 = std::string(data, len);
        }
    };
    writeCallbacks["file"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_FILENAME) {
            LOG_DEBUG << "query database";
            filename = file.filename;
            auto ret = sqlConn->query<File>("select * from t_file where file_md5 = '" + file_md5 +
                                            "' and user_id = " + std::to_string(userId));
            if (!ret.empty()) {
                ret[0].file_name = filename;
                sqlConn->update(ret[0]);
                j["file_url"] = ret[0].file_url;
                isExist = true;
            } else {
                fileType = IMUtil::getFileType(filename);
                std::string randomFileName = IMUtil::generateRandomFileName(fileType);
                fileUrl = FILE_URL_PREFIX + std::to_string(userId) + "/" + randomFileName;
                filePath = UPLOAD_PATH + std::to_string(userId) + "/" + randomFileName;
                fd = open(filePath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
                if (fd < 0) {
                    LOG_DEBUG << "open file failed";
                }
            }
        } else if (flag == FLAG_CONTENT) {
            if (!isExist && fd > 0) {
                write(fd, data, len);
            }
        }
    };
    if (!request.parseMultiPartFormData(writeCallbacks)) {
        if (fd > 0 && access(filePath.c_str(), F_OK) == 0) {
            remove(filePath.c_str());
            close(fd);
        }
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    if (isExist) {
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "文件已存在")
    }
    if (fd < 0) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    fsync(fd);
    lseek(fd, 0, SEEK_SET);
    size_t bufferSize = 1024 * 1024;
    char* buf = new char[bufferSize];
    MD5 md5;
    ssize_t readSize;
    while (true) {
        readSize = read(fd, buf, bufferSize);
        if (readSize <= 0) {
            break;
        }
        md5.update(buf, static_cast<size_t>(readSize));
    }
    md5.finalize();
    delete[] buf;
    close(fd);
    std::string md5Str = md5.hexdigest();
    if (md5Str != file_md5) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    struct stat statBuf {};
    size_t fileSize;
    if (::stat(filePath.c_str(), &statBuf) == 0) {
        fileSize = statBuf.st_size;
    } else {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    File file(fileType, filename, fileUrl, static_cast<long>(fileSize), md5Str, userId, time(nullptr));
    sqlConn->insert(file);
    j["file_url"] = fileUrl;
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "上传成功")
}

void FileController::shardUpload(HttpRequest& request, HttpResponse& response) {
    LOG_DEBUG << "FileController::shardUpload";
    json j;
    auto token = request.getHeaderValue("token");
    auto userId = JwtUtil::parseToken(token);
    if (userId == -1) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "无效token")
    }
    GET_SQL_CONN_H(sqlConn)
    std::string su_id;
    int shard_index;
    int shard_size;
    int shard_count;
    std::string file_md5;
    std::string filename;
    std::string fileType;
    std::string randomFileName;
    std::string filePath;
    bool isExist = false;
    int fd1 = -1;
    MultiPartWriteCallbackMap writeCallbacks;
    writeCallbacks["su_id"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_CONTENT) {
            su_id = std::string(data, len);
        }
    };
    writeCallbacks["shard_index"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_CONTENT) {
            shard_index = std::stoi(std::string(data, len));
        }
    };
    writeCallbacks["shard_size"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_CONTENT) {
            shard_size = std::stoi(std::string(data, len));
        }
    };
    writeCallbacks["shard_count"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_CONTENT) {
            shard_count = std::stoi(std::string(data, len));
        }
    };
    writeCallbacks["file_md5"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_CONTENT) {
            file_md5 = std::string(data, len);
        }
    };
    writeCallbacks["file"] = [&](const MultipartFormData& file, const char* data, size_t len, int flag) {
        if (flag == FLAG_FILENAME) {
            filename = file.filename;
            auto ret = sqlConn->query<File>("select * from t_file where file_md5 = '" + file_md5 +
                                            "' and user_id = " + std::to_string(userId));
            if (!ret.empty()) {
                if (shard_index == 1) {
                    ret[0].file_name = filename;
                    sqlConn->update(ret[0]);
                }
                j["complete"] = 1;
                j["file_url"] = ret[0].file_url;
                isExist = true;
            } else {
                fileType = IMUtil::getFileType(filename);
                randomFileName = su_id + "." + fileType;
                filePath = UPLOAD_PATH + std::to_string(userId) + "/" + randomFileName;
                fd1 = open(filePath.c_str(), O_CREAT | O_WRONLY, 0644);
                if (fd1 < 0) {
                    LOG_DEBUG << "open file failed";
                }
                lseek(fd1, shard_index * shard_size, SEEK_SET);
            }
        } else if (flag == FLAG_CONTENT) {
            if (!isExist && fd1 > 0) {
                write(fd1, data, len);
            }
        }
    };
    if (!request.parseMultiPartFormData(writeCallbacks)) {
        if (fd1 > 0 && access(filePath.c_str(), F_OK) == 0) {
            remove(filePath.c_str());
            close(fd1);
        }
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
    if (isExist) {
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "文件已存在")
    }
    if (fd1 < 0) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
    }
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
        int fd2 = open(filePath.c_str(), O_RDONLY, 0644);
        if (fd2 < 0) {
            redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
            RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
        }
        size_t bufSize = 1024 * 1024;
        char* buf = new char[bufSize];
        MD5 md5;
        ssize_t readSize;
        while (true) {
            readSize = read(fd2, buf, bufSize);
            if (readSize <= 0) {
                break;
            }
            md5.update(buf, static_cast<size_t>(readSize));
        }
        md5.finalize();
        std::string md5Str = md5.hexdigest();
        if (md5Str != file_md5) {
            LOG_DEBUG << "md5 not equal";
            redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
            RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
        }
        struct stat statBuf {};
        size_t fileSize;
        if (::stat(filePath.c_str(), &statBuf) == 0) {
            fileSize = statBuf.st_size;
        } else {
            redisConn_->del(REDIS_KEY_SHARD_UPLOAD_FILE_MUTEX + su_id);
            RETURN_RESPONSE(HTTP_ERROR_CODE, "文件上传失败")
        }
        std::string fileUrl = FILE_URL_PREFIX + std::to_string(userId) + "/" + randomFileName;
        File file(fileType, filename, fileUrl, static_cast<long>(fileSize), md5Str, userId, time(nullptr));
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
