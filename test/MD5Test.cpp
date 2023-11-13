#include <fcntl.h>
#include <sys/stat.h>

#include <csignal>
#include <iostream>
#include <util/MD5.hpp>

int main() {
    std::string filePath =
        "/home/linhaojun/cpp-code/cooper-im-server/static/upload/1/2ff3499d30b1412b894c227203a43bb2.jpg";
    struct stat statbuf {};
    size_t fileSize;
    if (::stat(filePath.c_str(), &statbuf) == 0) {
        fileSize = statbuf.st_size;
    } else {
        std::cout << "failed!" << std::endl;
    }
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cout << "failed!" << std::endl;
    }
    std::string fileContent(fileSize, '\0');
    read(fd, fileContent.data(), fileSize);
    close(fd);
    std::cout << MD5(fileContent).hexdigest() << std::endl;
    return 0;
}
