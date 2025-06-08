#pragma once
#include <string>

namespace IO_utility {

class FileReader {
public:
    std::string ReadTextFile(const std::string &path) const;
};

class FileDeleter {
public:
    void DeleteFile(const std::string &path) const;
};

class FileIO {
public:
    static std::string ReadTextFile(const std::string &path) {
        return FileReader{}.ReadTextFile(path);
    }
    static void DeleteFile(const std::string &path) {
        FileDeleter{}.DeleteFile(path);
    }
};

inline std::string readFile(const char *path) {
    return FileIO::ReadTextFile(path);
}

inline void deleteFile(const char *path) {
    FileIO::DeleteFile(path);
}

} // namespace IO_utility
