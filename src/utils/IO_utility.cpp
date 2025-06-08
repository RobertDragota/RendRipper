#include "IO_utility.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <stdexcept>

namespace IO_utility {

std::string FileReader::ReadTextFile(const std::string &path) const
{
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error(std::string("Cannot open file: ") + path);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void FileDeleter::DeleteFile(const std::string &path) const
{
    if (std::remove(path.c_str()) != 0) {
        throw std::runtime_error(std::string("Cannot delete file: ") + path);
    }
}

} // namespace IO_utility
