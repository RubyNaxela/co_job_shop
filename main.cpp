#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include "heuristics.hpp"
#include "platform.hpp"
#include "schedule.hpp"
#include "range.hpp"
#include "timer.hpp"

// #define VERBOSE

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

typedef uint64_t hash_t;

hash_t hash_insertion_order(const std::vector<js::task_coordinates>& insertion_order) {
    std::hash<uint64_t> hash_function;
    hash_t hash = insertion_order.size();
    for (auto& coordinates : insertion_order)
        hash ^= hash_function(uint64_t(coordinates.job_id) << 32 | coordinates.task_id)
                + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

bool swappable(size_t r1, size_t r2, const std::vector<js::task_coordinates>& insertion_order) {
    const js::task_coordinates t1 = insertion_order[r1], t2 = insertion_order[r2];
    if (t1.job_id == t2.job_id) return false;
    if (r1 > r2) std::swap(r1, r2);
    return std::none_of(insertion_order.begin() + ptrdiff_t(r1) + 1, insertion_order.begin() + ptrdiff_t(r2),
                        [&](js::task_coordinates t) { return t.job_id == t1.job_id or t.job_id == t2.job_id; });
}

bool insertion_order_tested(std::vector<js::task_coordinates> insertion_order, const std::set<hash_t>& tested_orders,
                            size_t t1, size_t t2) {
    std::swap(insertion_order[t1], insertion_order[t2]);
    return tested_orders.find(hash_insertion_order(insertion_order)) != tested_orders.end();
}

int main(int argc, char** argv) {

    zr::timer<zr::precision::ms> master_timer;
    master_timer.start();

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

    zr::timer<zr::precision::ms> algorithm_timer;
    if (measure_time) algorithm_timer.start();

    for (uint32_t it = 0; it < iterations; it++) {

        struct {
            js::basic_schedule schedule;
            std::vector<js::task_coordinates> insertion_order;
            std::string summary;
            std::optional<js::time32_t> time;
        } solution;

        const js::heuristic heuristics[] = {
                {js::do_nothing, js::heuristics::pass},
                {js::reverse,    js::heuristics::pass},
                {js::sort,       js::heuristics::stachu_ascending},
                {js::sort,       js::heuristics::stachu_descending}
        };

        for (const auto& heuristic : heuristics) {

            js::schedule schedule(js::dataset(data_string, limit));
            std::vector<js::task*> insertion_order = schedule.schedule_jobs(heuristic);

            if (not solution.time.has_value() or schedule.longest_timeline() < solution.time) {
                if (display_gantt_chart) solution.schedule = *dynamic_cast<js::basic_schedule*>(&schedule);
                solution.insertion_order.clear();
                std::transform(insertion_order.begin(), insertion_order.end(), std::back_inserter(solution.insertion_order),
                               [](const js::task* t) { return t->coordinates(); });
                solution.summary = schedule.summary();
                solution.time = schedule.longest_timeline();
            }
        }

#ifdef VERBOSE
        std::cout << "Initial solution:\n" << solution.summary << std::endl;
#endif

        // ==========================================[ Local search goes here ]==========================================

        js::dataset data(data_string, limit);

        std::vector<js::task_coordinates> insertion_order = solution.insertion_order;
        std::set<hash_t> tested_orders;
        tested_orders.insert(hash_insertion_order(insertion_order));

        struct swap {
            size_t t1 = 0, t2 = 0;
            js::time32_t time = js::interval::infinity;
        };

        constexpr uint64_t time_limit = 0.033 * 60 * 1000;
        while (master_timer.get_measured_time() < time_limit) {
#ifdef VERBOSE
            std::cout << "Elapsed time: " << master_timer.get_measured_time() << master_timer.unit() << std::endl;
#endif

            size_t all_pairs = 0, swappable_pairs = 0;
            std::vector<swap> best_swaps;
            for (size_t i = 0; i < insertion_order.size() - 1; i++) {
                for (size_t j = i + 1; j < insertion_order.size(); j++) {
                    all_pairs++;
                    if (swappable(i, j, insertion_order) and
                        not insertion_order_tested(insertion_order, tested_orders, i, j)) {
                        swappable_pairs++;
                        swap swap;
                        swap.t1 = i;
                        swap.t2 = j;
                        std::swap(insertion_order[i], insertion_order[j]);
                        js::schedule local_schedule(data);
                        local_schedule.schedule_jobs(insertion_order);
                        swap.time = local_schedule.longest_timeline();
                        if (best_swaps.empty() or best_swaps[0].time > swap.time) {
                            best_swaps.clear();
                            best_swaps.push_back(swap);
                        } else if (best_swaps[0].time == swap.time)
                            best_swaps.push_back(swap);
                        tested_orders.insert(hash_insertion_order(insertion_order));

                        std::swap(insertion_order[i], insertion_order[j]);
                        for (js::job& job : data.jobs) job.last_scheduled_time = 0;
                    }
                }
            }
            std::optional<swap> best_swap = not best_swaps.empty()
                                            ? std::make_optional(best_swaps[best_swaps.size() / 2])
                                            : std::nullopt;
#ifdef VERBOSE
            std::cout << "All pairs: " << all_pairs << ", swappable pairs: " << swappable_pairs << ", banned orders: "
                      << tested_orders.size() << std::endl;
#endif
            if (best_swap.has_value()) {
                std::swap(insertion_order[best_swap.value().t1], insertion_order[best_swap.value().t2]);
                tested_orders.insert(hash_insertion_order(insertion_order));
                js::schedule local_schedule(data);
                local_schedule.schedule_jobs(insertion_order);
                if (local_schedule.longest_timeline() < solution.time) {
                    solution.schedule = *dynamic_cast<js::basic_schedule*>(&local_schedule);
                    solution.insertion_order = insertion_order;
                    solution.summary = local_schedule.summary();
                    solution.time = local_schedule.longest_timeline();
#ifdef VERBOSE
                    std::cout << "================ New best solution found: " << solution.time.value() << "================\n";
#endif
                }
#ifdef VERBOSE
                else std::cout << "No improvement so far; current time: " << local_schedule.longest_timeline() << std::endl;

                std::cout << "Insertion order: ";
                for (const auto& task : insertion_order)
                    std::cout << "j" << task.job_id << "::t" << task.task_id << " ";
                std::cout << std::endl << "(hash: " << hash_insertion_order(insertion_order) << ")" << std::endl;
#endif
            } else {
#ifdef VERBOSE
                std::cout << "No available solution modifications" << std::endl;
#endif
                break;
            }

            for (js::job& job : data.jobs) job.last_scheduled_time = 0;

            js::schedule schedule(data);
            schedule.schedule_jobs(insertion_order);
        }

        // ==============================================================================================================

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
            if (display_gantt_chart) std::cout << std::endl << solution.schedule.gantt_chart() << std::endl;
        }
    }

    return 0;
}
