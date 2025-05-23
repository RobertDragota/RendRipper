
#include "IO_utility.h"
#include <fstream>
#include <sstream>

namespace IO_utility {

    std::string readFile(const char *path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error(std::string("Cannot open file: ") + path);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

}