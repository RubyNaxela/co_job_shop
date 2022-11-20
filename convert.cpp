#include <fstream>
#include <iostream>
#include <vector>
#include "dataset.hpp"
#include "platform.hpp"

void discard_next(std::istream& file, size_t count) {
    std::string discard;
    for (size_t i = 0; i < count; i++) file >> discard;
}

void convert(const std::string& input, const std::string& output) {

    js::create_directory(output);
    std::ifstream input_file(input);
    std::ofstream output_file(output);

    if (not input_file.is_open()) throw std::runtime_error("Could not open file " + std::string(input));
    if (not output_file.is_open()) throw std::runtime_error("Could not open file " + std::string(output));

    js::id32_t jobs_count = 0, machines_count = 0;
    size_t tasks_count = 0;
    std::vector<js::time32_t> times;
    std::vector<js::id32_t> machines;

    while (not input_file.eof()) {

        input_file >> jobs_count >> machines_count;
        tasks_count = jobs_count * machines_count;

        times.reserve(tasks_count);
        discard_next(input_file, 5);
        for (size_t i = 0; i < tasks_count; i++) input_file >> times[i];

        machines.reserve(tasks_count);
        discard_next(input_file, 1);
        for (size_t i = 0; i < tasks_count; i++) input_file >> machines[i];
    }
    input_file.close();

    output_file << jobs_count << " " << machines_count << std::endl;
    for (size_t i = 0; i < tasks_count;) {
        output_file << machines[i] % machines_count << " " << times[i] << " ";
        if (++i % machines_count == 0) output_file << std::endl;
    }
    output_file.close();
}

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Convert a tailard format instance to orlib format" << std::endl;
        std::cout << "Usage: " << js::extract_file_name(argv[0]) << " <input> <output> [-d]" << std::endl;
        return 1;
    }

    if (argc > 3 and std::string(argv[3]) == "-d") {
        const std::string input_directory = argv[1], output_directory = argv[2];
        js::create_directory(output_directory);
        for (const auto& entry : std::filesystem::directory_iterator(input_directory)) {
            const std::filesystem::path& path = entry.path();
            const std::string input = path.string();
            const std::string output = (std::ostringstream{} << output_directory << js::path_sep
                                                             << path.filename().string()).str();
            convert(input, output);
        }
    } else convert(argv[1], argv[2]);
}