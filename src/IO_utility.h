#pragma once
#include <string>

namespace IO_utility {

class FileIO {
public:
    static std::string ReadTextFile(const std::string &path);
    static void DeleteFile(const std::string &path);
};

// Backwards compatible wrappers
inline std::string readFile(const char *path) {
    return FileIO::ReadTextFile(path);
}

inline void deleteFile(const char *path) {
    FileIO::DeleteFile(path);
}

} // namespace IO_utility



