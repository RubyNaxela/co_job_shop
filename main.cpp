#include <algorithm>
#include <iostream>
#include "dataset.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

void alex_algorithm(js::dataset& data, js::schedule& schedule) {
    for (js::task& task : data.tasks) for (js::sub_task& sub_task : task.sequence) schedule.add_sub_task(sub_task);
}

void stachu_algorithm(js::dataset& data, js::schedule& schedule) {
    for (int i = 0; i < data.tasks[0].sequence.size(); i++) {
        std::sort(data.tasks.begin(), data.tasks.end(),
                  [=](const js::task& a, const js::task& b) { return a.sequence[i].duration > b.sequence[i].duration; });
        for (auto& task : data.tasks) schedule.add_sub_task(task.sequence[i]);
    }
}

int main(int argc, char** argv) {
    std::string data_file = "data_from_mary.txt";
    if (argc > 1) data_file = std::string(argv[1]);
    js::dataset data = js::dataset::from_file(data_file.c_str());
    js::schedule schedule(data);
    // alex_algorithm(data, schedule);
    stachu_algorithm(data, schedule);
    std::cout << schedule << std::endl;
    std::cout << schedule.summary() << std::endl;
    return 0;
}
