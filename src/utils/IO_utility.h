#pragma once
#include <string>
#include <cstdio>

namespace IO_utility {

/**
 * @brief Simple synchronous text file reader.
 */
class FileReader {
public:
    std::string ReadTextFile(const std::string &path) const;
};

class FileIO {
public:
    /** Convenience wrapper for FileReader. */
    static std::string ReadTextFile(const std::string &path) {
        return FileReader{}.ReadTextFile(path);
    }
};

/// Convenience free function to read a text file.
inline std::string readFile(const char *path) {
    return FileIO::ReadTextFile(path);
}

/// Delete a file from disk.
inline void deleteFile(const char *path) {
    std::remove(path);
}

} // namespace IO_utility
