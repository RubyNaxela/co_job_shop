#include <array>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <stdexcept>

#ifndef JOB_SHOP_PLATFORM
#define JOB_SHOP_PLATFORM

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define POSIX
#else
#define WINDOZE
#endif

#ifdef WINDOZE
#define EXECUTABLE_PREFIX ""
#define pipe_open _popen
#define pipe_close _pclose
#else
#define EXECUTABLE_PREFIX "./"
#define pipe_open popen
#define pipe_close pclose
#endif

namespace js {

    static inline char path_sep = std::filesystem::path::preferred_separator;

    std::string extract_file_name(const std::string& path) {
        size_t last_slash = path.find_last_of(path_sep);
        if (last_slash == std::string::npos) return path;
        return path.substr(last_slash + 1);
    }

    void create_directory(const std::string& path) {
        std::ostringstream path_builder;
        path_builder << "." << path_sep << path;
        std::filesystem::path directory = path_builder.str();
        directory.remove_filename();
        if (not std::filesystem::exists(directory)) std::filesystem::create_directories(directory);
    }

    std::string execute(const std::string& command) {
        std::array<char, 128> buffer{};
        std::string result;
        std::unique_ptr<FILE, decltype(&pipe_close)> pipe(pipe_open(command.c_str(), "r"), pipe_close);
        if (!pipe) throw std::runtime_error("execute() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
        return result;
    }
}

#endif //JOB_SHOP_PLATFORM
