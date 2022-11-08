#include <iostream>
#include <algorithm>
#include "dataset.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main() {
    js::dataset data = js::dataset::from_file("data.txt");
    js::schedule schedule;
    //i<data.tasks.subtasks.size
    for (int i=0; i<data.tasks[0].sequence.size();i++)
    {
        std::sort(data.tasks.begin(),data.tasks.end(),[=](const js::task& a, const js::task& b)
        { return a.sequence[i].duration < b.sequence[i].duration; });

        for(int j=0; j<data.tasks.size();j++)
        {
            schedule.add_sub_task(data.tasks[j].sequence[i]);
        }
    }
    return 0;
}
