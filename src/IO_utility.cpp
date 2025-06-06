
#include "IO_utility.h"

#include <fstream>
#include <sstream>
#include <cstdio>

namespace IO_utility {

std::string FileIO::ReadTextFile(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error(std::string("Cannot open file: ") + path);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void FileIO::DeleteFile(const std::string &path) {
    if (std::remove(path.c_str()) != 0) {
        throw std::runtime_error(std::string("Cannot delete file: ") + path);
    }
}

} // namespace IO_utility
