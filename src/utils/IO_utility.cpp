#include "IO_utility.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * @file IO_utility.cpp
 * @brief Simple file reading helpers.
 */

namespace IO_utility {

/** Read entire file contents into a string. */
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

} // namespace IO_utility
