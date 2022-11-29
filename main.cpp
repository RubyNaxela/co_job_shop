#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include "heuristics.hpp"
#include "platform.hpp"
#include "schedule.hpp"
#include "timer.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main(int argc, char** argv) {

    std::string data_path = "data.txt";
    bool display_gantt_chart = false, measure_time = false;
    std::string output_path;
    uint16_t iterations = 1, limit = 0;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-d") data_path = argv[++i];
        if (std::string(argv[i]) == "-l") limit = std::stol(argv[++i]);
        if (std::string(argv[i]) == "-g") display_gantt_chart = true;
        if (std::string(argv[i]) == "-o") output_path = argv[++i];
        if (std::string(argv[i]) == "-t") measure_time = true;
        if (std::string(argv[i]) == "-r") iterations = std::stol(argv[++i]);
    }

    std::ifstream data_file(data_path);
    if (not data_file.is_open()) throw std::runtime_error("Could not open file " + data_path);
    std::ostringstream buffer;
    buffer << data_file.rdbuf();
    std::string data_string = buffer.str();

    js::timer<js::precision::us> timer;
    if (measure_time) timer.start();

    for (uint32_t it = 0; it < iterations; it++) {

        struct {
            js::basic_schedule schedule;
            std::string summary;
            std::optional<js::time32_t> time;
        } solution;

        const js::heuristic heuristics[] = {{js::do_nothing, js::heuristics::pass},
                                            {js::reverse, js::heuristics::pass},
                                            {js::sort, js::heuristics::stachu_ascending},
                                            {js::sort, js::heuristics::stachu_descending}};

        for (const auto& heuristic : heuristics) {

            js::dataset data;
            data.load_from_memory(data_string, limit);

            js::schedule schedule(data);
            schedule.schedule_jobs(heuristic);

            if (not solution.time.has_value() or schedule.longest_timeline() < solution.time) {
                if (display_gantt_chart) solution.schedule = *dynamic_cast<js::basic_schedule*>(&schedule);
                solution.summary = schedule.summary();
                solution.time = schedule.longest_timeline();
            }
        }

        if (measure_time) {
            timer.stop();
            std::cout << timer.get_measured_time() << std::endl;
            timer.start();
        } else {
            if (not output_path.empty()) {
                js::create_directory(output_path);
                std::ofstream file_out(output_path);
                file_out << solution.summary;
                file_out.close();
            } else std::cout << solution.summary;
            if (display_gantt_chart) std::cout << std::endl << solution.schedule.gantt_chart() << std::endl;
        }
    }

    return 0;
}
