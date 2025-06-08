#pragma once
#include <string>
#include <cstdio>

namespace IO_utility {

class FileReader {
public:
    std::string ReadTextFile(const std::string &path) const;
};

class FileIO {
public:
    static std::string ReadTextFile(const std::string &path) {
        return FileReader{}.ReadTextFile(path);
    }
};

inline std::string readFile(const char *path) {
    return FileIO::ReadTextFile(path);
}

inline void deleteFile(const char *path) {
    std::remove(path);
}

} // namespace IO_utility
