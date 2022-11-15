#include <iostream>
#include "dispatcher.hpp"
#include "heuristics.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main(int argc, char** argv) {

    std::string data_file = "data.txt";
    bool display_gantt_chart = false;
    std::string heuristic = "stachu_descending";

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-d") data_file = argv[++i];
        if (std::string(argv[i]) == "-g") display_gantt_chart = true;
        if (std::string(argv[i]) == "-h") heuristic = argv[++i];
    }

    js::dataset data;
    data.load_from_file(data_file.c_str());
    js::schedule schedule(data);
    js::schedule_jobs(data, schedule, js::heuristics::get_by_name(heuristic));

    if (display_gantt_chart) std::cout << schedule.gantt_chart() << std::endl;
    std::cout << schedule.summary() << std::endl;
    return 0;
}
