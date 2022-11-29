#include <filesystem>
#include <iostream>
#include <sstream>
#include "platform.hpp"

void run_test(const std::string& solver_executable, const std::string& checker_executable,
              const std::string& data_directory, const std::string& output_directory) {
    for (const auto& entry : std::filesystem::directory_iterator(data_directory)) {
        const std::string input = entry.path().string();
        const std::string output = output_directory + js::path_sep + entry.path().filename().string();
        js::execute((std::ostringstream{} << EXECUTABLE_PREFIX << solver_executable
                                          << " -d " << input << " -o " << output).str());
        if (not checker_executable.empty()) {
            const std::string feedback = js::execute((std::ostringstream{} << EXECUTABLE_PREFIX << checker_executable
                                                                           << " " << input << " " << output).str());
            if (feedback.find("OK") != std::string::npos) std::cout << "[ OK ] " << input << " -> " << output << std::endl;
            else std::cerr << "[FAIL] " << input << " -> " << output << "\n" << feedback << std::endl;
        }
    }
}

int main(int argc, char** argv) {

    if (argc < 4) {
        const std::string executable = js::extract_file_name(argv[0]);
        std::cout << "Perform automatic test on instances from a directory" << std::endl;
        std::cout << "Usage: " << executable
                  << " <solver_executable> <data_directory> <output_directory> [checker_executable]" << std::endl;
        return 1;
    }
    run_test(argv[1], argc > 4 ? argv[4] : "", argv[2], argv[3]);
}