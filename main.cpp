#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
#include "heuristics.hpp"
#include "platform.hpp"
#include "schedule.hpp"
#include "random.hpp"
#include "range.hpp"
#include "timer.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

bool should_redraw(size_t r1, size_t r2, const std::vector<js::task*>& insertion_order, size_t machines_count) {

    if (r1 == r2 or insertion_order[r1]->parent.id == insertion_order[r2]->parent.id) return true;

    const size_t swap_candidates[2] = {r1, r2};
    for (size_t c : swap_candidates) {
        const js::task* candidate = insertion_order[c];
        const auto same_job_as_c = [&](const js::task* t) { return t->parent.id == candidate->parent.id; };
        if (candidate->sequence_order < machines_count - 1) {
            const auto t = *std::find_if(insertion_order.begin() + ptrdiff_t(c + 1), insertion_order.end(), same_job_as_c);
            if (t->sequence_order != candidate->sequence_order + 1) return true;
        } else {
            const auto t = *std::find_if(insertion_order.rend() - ptrdiff_t(c + 1), insertion_order.rend(), same_job_as_c);
            if (t->sequence_order != candidate->sequence_order - 1) return true;
        }
    }
    return false;
}

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

        const js::heuristic heuristics[] = {js::heuristics::alex_forward, js::heuristics::alex_backward,
                                            js::heuristics::stachu_ascending, js::heuristics::stachu_descending};

        for (const auto& heuristic : heuristics) {

            js::dataset data;
            data.load_from_memory(data_string, limit);

            js::schedule schedule(data);
            std::vector<js::task*> insertion_order = schedule.schedule_jobs(heuristic);
            std::cout << "\nInsertion order: ";
            for (const auto& task : insertion_order)
                std::cout << "j" << task->parent.id << "::t" << task->sequence_order << " ";
            std::cout << "\n\n" << schedule.gantt_chart() << std::endl;

            size_t random_operation1 = -1, random_operation2 = -1;
            const js::range<size_t> tasks_range = js::make_range<size_t, js::closed_open>(0, insertion_order.size());
            while (should_redraw(random_operation1, random_operation2, insertion_order, data.machines_count)) {
                random_operation1 = js::random_number<size_t>(tasks_range);
                random_operation2 = js::random_number<size_t>(tasks_range);
            }
            std::swap(insertion_order[random_operation1], insertion_order[random_operation2]);

            for (js::job& job : data.jobs) job.last_scheduled_time = 0;

            js::schedule schedule2(data);
            schedule2.schedule_jobs(insertion_order);
            std::cout << "Insertion order: ";
            for (const auto& task : insertion_order)
                std::cout << "j" << task->parent.id << "::t" << task->sequence_order << " ";
            std::cout << "\n\n" << schedule2.gantt_chart() << std::endl;

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
