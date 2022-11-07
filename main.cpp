#include <iostream>
#include "dataset.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main() {
    js::dataset data = js::dataset::from_file("short_data.txt");
    js::schedule schedule(data.machine_count);

    for (js::task& t : data.tasks) {
//        std::cout << "scheduling task #" << t.id << std::endl;
        for (js::sub_task& st : t.sequence) schedule.add_sub_task(st);
//        std::cout << schedule << std::endl;
    }
    std::cout << schedule << std::endl;

    return 0;
}
