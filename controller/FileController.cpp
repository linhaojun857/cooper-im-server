#include "FileController.hpp"

#include <cooper/util/Logger.hpp>
#include <fstream>
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
    auto md5 = MD5(fileData.content).hexdigest();
    auto ret = sqlConn->query<File>("select * from t_file where file_md5 = '" + md5 +
                                    "' and user_id = " + std::to_string(userId));
    if (!ret.empty()) {
        ret[0].file_name = fileData.filename;
        sqlConn->update(ret[0]);
        RETURN_RESPONSE(HTTP_SUCCESS_CODE, "上传成功")
    }
    std::string fileType = IMUtil::getFileType(fileData.filename);
    std::string randomFileName = IMUtil::generateRandomFileName(fileType);
    std::string fileUrl = FILE_URL_PREFIX + std::to_string(userId) + "/" + randomFileName;
    std::ofstream fs;
    fs.open(UPLOAD_PATH + std::to_string(userId) + "/" + randomFileName, std::ios::out | std::ios::trunc);
    if (!fs) {
        RETURN_RESPONSE(HTTP_ERROR_CODE, "上传失败")
    }
    fs.write(fileData.content.c_str(), static_cast<std::streamsize>(fileData.content.size()));
    fs.close();
    File file(fileType, fileData.filename, fileUrl, static_cast<long>(fileData.content.size()), md5, userId,
              time(nullptr));
    sqlConn->insert(file);
    RETURN_RESPONSE(HTTP_SUCCESS_CODE, "上传成功")
}
