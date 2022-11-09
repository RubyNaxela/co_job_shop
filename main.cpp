#include <algorithm>
#include <iostream>
#include <numeric>
#include "dataset.hpp"
#include "heuristics.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

namespace js {

    void schedule_tasks(js::dataset& data, js::schedule& schedule, const js::heuristic& heuristic) {
        const size_t sequence_length = data.tasks[0].sequence.size();
        for (size_t i = 0; i < sequence_length; i++) {
            std::vector<js::task*> tasks_order(data.tasks.size());
            std::iota(tasks_order.begin(), tasks_order.end(), &data.tasks[0]);
            std::sort(tasks_order.begin(), tasks_order.end(),
                      [=](const js::task* a, const js::task* b) { return heuristic(a, b, i); });
            for (const auto& task : tasks_order) schedule.add_sub_task(task->sequence[i]);
        }
    }
}

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
    js::schedule_tasks(data, schedule, js::heuristics::get_by_name(heuristic));

    if (display_gantt_chart) std::cout << schedule.gantt_chart() << std::endl;
    std::cout << schedule.summary() << std::endl;
    return 0;
}
