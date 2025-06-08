#pragma once
#include <cstdio>
#include <string>

// Simple RAII wrapper for popen/_popen
// Opens a read-only pipe and ensures it is closed.
// Use get() to read from the pipe and close() to
// retrieve the process return code.
class Pipe {
public:
    explicit Pipe(const std::string &cmd) {
#ifdef _WIN32
        pipe_ = _popen(cmd.c_str(), "r");
#else
        pipe_ = popen(cmd.c_str(), "r");
#endif
    }
    ~Pipe() { close(); }

    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;

    bool valid() const { return pipe_ != nullptr; }
    FILE* get() const { return pipe_; }

    int close() {
        if (!pipe_) return -1;
#ifdef _WIN32
        int ret = _pclose(pipe_);
#else
        int ret = pclose(pipe_);
#endif
        pipe_ = nullptr;
        return ret;
    }

private:
    FILE* pipe_ = nullptr;
};
