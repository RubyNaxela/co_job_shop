#include <fstream>
#include <iostream>
#include <stdexcept>
#include "basic_search.hpp"
#include "local_search.hpp"
#include "platform.hpp"
#include "solution.hpp"
#include "timer.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main(int argc, char** argv) {

    zr::timer<zr::precision::ms> master_timer;
    master_timer.start();

    std::string data_path = "data.txt";
    bool display_gantt_chart = false, measure_time = false;
    std::string output_path;
    uint16_t iterations = 1, limit = 0, time_constraint = 15;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-c") time_constraint = std::stol(argv[++i]);
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

    zr::timer<zr::precision::ms> algorithm_timer;
    if (measure_time) algorithm_timer.start();

    for (uint32_t it = 0; it < iterations; it++) {

        js::solution solution = js::find_initial_solution(data_string, limit, display_gantt_chart);
        js::local_search(solution, master_timer, time_constraint * 1000);

        if (measure_time) {
            algorithm_timer.stop();
            std::cout << algorithm_timer.get_measured_time() << algorithm_timer.unit() << std::endl;
            algorithm_timer.start();
        } else {
            if (not output_path.empty()) {
                js::create_directory(output_path);
                std::ofstream file_out(output_path);
                file_out << solution.summary;
                file_out.close();
            } else std::cout << solution.summary;
            if (display_gantt_chart) std::cout << std::endl << solution.gantt_chart() << std::endl;
        }
    }

    return 0;
}
