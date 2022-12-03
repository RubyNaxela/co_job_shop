#ifndef JOB_SHOP_LOCAL_SEARCH
#define JOB_SHOP_LOCAL_SEARCH

#include "avl_tree_set.hpp"
#include "dataset.hpp"
#include "schedule.hpp"
#include "timer.hpp"

namespace js {

    class local_search_environment {

        dataset data;
        solution& solution;
        task_order insertion_order;
        zr::avl_tree_set<hash_t> tested_orders;

        bool swappable(size_t r1, size_t r2) {
            const js::task_coordinates t1 = insertion_order[r1], t2 = insertion_order[r2];
            if (t1.job_id == t2.job_id) return false;
            if (r1 > r2) std::swap(r1, r2);
            return std::none_of(insertion_order.begin() + ptrdiff_t(r1) + 1, insertion_order.begin() + ptrdiff_t(r2),
                                [&](js::task_coordinates t) { return t.job_id == t1.job_id or t.job_id == t2.job_id; });
        }

        bool insertion_order_tested(size_t t1, size_t t2) {
            std::swap(insertion_order[t1], insertion_order[t2]);
            const bool tested = tested_orders.contains(insertion_order);
            std::swap(insertion_order[t1], insertion_order[t2]);
            return tested;
        }

        std::optional<task_order::swap> find_best_swap() {
            std::vector<task_order::swap> best_swaps;
            for (size_t t1 = 0; t1 < insertion_order.size() - 1; t1++) {
                for (size_t t2 = t1 + 1; t2 < insertion_order.size(); t2++) {
                    if (swappable(t1, t2) and not insertion_order_tested(t1, t2)) {
                        task_order::swap swap(t1, t2);
                        std::swap(insertion_order[t1], insertion_order[t2]);
                        schedule local_schedule(data);
                        local_schedule.schedule_jobs(insertion_order);
                        swap.time = local_schedule.longest_timeline();
                        if (best_swaps.empty() or swap.time < best_swaps[0].time) {
                            best_swaps.clear();
                            best_swaps.push_back(swap);
                        } else if (best_swaps[0].time == swap.time)
                            best_swaps.push_back(swap);
                        tested_orders.insert(insertion_order);
                        std::swap(insertion_order[t1], insertion_order[t2]);
                        for (job& job : data.jobs) job.last_scheduled_time = 0;
                    }
                }
            }
            return not best_swaps.empty() ? std::make_optional(best_swaps[best_swaps.size() / 2]) : std::nullopt;
        }

        void accept_solution(const schedule& local_schedule) {
            solution.schedule = *dynamic_cast<const basic_schedule*>(&local_schedule);
            solution.insertion_order = insertion_order;
            solution.summary = local_schedule.summary();
            solution.time = local_schedule.longest_timeline();
        }

        void replace_if_better(const std::optional<task_order::swap>& best_swap) {
            std::swap(insertion_order[best_swap.value().t1], insertion_order[best_swap.value().t2]);
            tested_orders.insert(insertion_order);
            schedule local_schedule(data);
            local_schedule.schedule_jobs(insertion_order);
            if (local_schedule.longest_timeline() < solution.time) accept_solution(local_schedule);
        }

        void reset_job_times() {
            for (job& job : data.jobs) job.last_scheduled_time = 0;
        }

    public:

        explicit local_search_environment(js::solution& solution)
                : data(solution.data_source, solution.jobs_limit), solution(solution),
                  insertion_order(solution.insertion_order) {
            tested_orders.insert(insertion_order);
        }

        bool advance() {

            const std::optional<task_order::swap> best_swap = find_best_swap();
            if (best_swap.has_value()) replace_if_better(best_swap);
            else return false;

            reset_job_times();
            schedule schedule(data);
            schedule.schedule_jobs(insertion_order);
            return true;
        }
    };

    void local_search(solution& solution, zr::timer<zr::precision::ms>& master_timer, uint64_t time_limit) {

        local_search_environment environment(solution);
        const uint64_t start_time = master_timer.get_measured_time();
        size_t iterations = 0;
        const auto average_iteration_time = [&]() { return (master_timer.get_measured_time() - start_time) / iterations; };

        do {
            const bool search_further = environment.advance();
            if (not search_further) break;
            iterations++;
        } while (master_timer.get_measured_time() < time_limit - average_iteration_time());
    }
}

#endif //JOB_SHOP_LOCAL_SEARCH
