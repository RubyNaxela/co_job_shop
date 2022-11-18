#include <iostream>
#include <fstream>
#include "dispatcher.hpp"
#include "heuristics.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main(int argc, char** argv) {

    std::string data_file = "data.txt";
    bool display_gantt_chart = false;
    //std::string heuristic = "stachu_descending";
    std::string path="results/new_results.txt";
    std::string heuristics[] = {"alex_forward","alex_backward","stachu_ascending","stachu_descending"};
    js::time32_t shortest_time = std::numeric_limits<js::time32_t>::max();
    std::fstream file;

    /*for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-d") data_file = argv[++i];
        if (std::string(argv[i]) == "-g") display_gantt_chart = true;
        if (std::string(argv[i]) == "-h") heuristic = argv[++i];
    }*/

    js::dataset data;
    data.load_from_file(data_file.c_str());

    for(int i=0;i<4;i++)
    {
        js::schedule schedule(data);
        js::schedule_jobs(data, schedule, js::heuristics::get_by_name(heuristics[i]));
        if(schedule.longest_timeline() < shortest_time)
        {
            file.open(path, std::ios::out);
            file<<schedule.summary()<<std::endl;
            file.close();
            shortest_time = schedule.longest_timeline();
        }
        if(i==3)
        {
            if(shortest_time == std::numeric_limits<js::time32_t>::max())
                std::cout << schedule.summary() << std::endl;
            else
            {
                file.open(path, std::ios::in);
                std::string line;
                while(std::getline(file,line)) std::cout<<line;
                file.close();
            }
        }
    }

    //if (display_gantt_chart) std::cout << schedule.gantt_chart() << std::endl;
    
    return 0;
}
