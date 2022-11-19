#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include "heuristics.hpp"
#include "platform.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main(int argc, char** argv) {

    std::string data_path = "data.txt";
    bool display_gantt_chart = false;
    std::string output_path;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-d") data_path = argv[++i];
        if (std::string(argv[i]) == "-g") display_gantt_chart = true;
        if (std::string(argv[i]) == "-o") output_path = argv[++i];
    }

    std::ifstream data_file(data_path);
    if (not data_file.is_open()) throw std::runtime_error("Could not open file " + data_path);
    std::ostringstream buffer;
    buffer << data_file.rdbuf();
    std::string data_string = buffer.str();

    std::optional<js::time32_t> shortest_time;
    std::string solution;

    const js::heuristic heuristics[] = {js::heuristics::alex_forward, js::heuristics::alex_backward,
                                        js::heuristics::stachu_ascending, js::heuristics::stachu_descending};
    for (const auto& heuristic : heuristics) {

        js::dataset data;
        data.load_from_memory(data_string);

        js::schedule schedule(data);
        schedule.schedule_jobs(heuristic);

        if (not shortest_time.has_value() or schedule.longest_timeline() < shortest_time) {
            solution = schedule.summary();
            shortest_time = schedule.longest_timeline();
        }
    }

    if (not output_path.empty()) {
        js::create_directory(output_path);
        std::ofstream file_out(output_path);
        file_out << solution;
        file_out.close();
    }
    std::cout << solution;

    // if (display_gantt_chart) std::cout << schedule->gantt_chart() << std::endl;

    return 0;
}
