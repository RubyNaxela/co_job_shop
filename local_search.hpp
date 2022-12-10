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
        size_t tasks_count;
        zr::avl_tree_set<hash_t> tested_orders;

        bool insertion_order_tested(task_order::deferred_swap swap) {
            swap.commit();
            const bool tested = tested_orders.contains(insertion_order);
            swap.revert();
            return tested;
        }

        std::optional<task_order::deferred_swap> find_best_swap() {
            std::vector<task_order::deferred_swap> best_swaps;
            schedule local_schedule(data);
            for (size_t t1 = 0; t1 < tasks_count - 1; t1++) {
                for (size_t t2 = t1 + 1; t2 < tasks_count; t2++) {
                    task_order::deferred_swap swap = insertion_order.make_swap(t1, t2);
                    if (swap.allowed() and not insertion_order_tested(swap)) {
                        swap.commit();
                        local_schedule.schedule_jobs(insertion_order);
                        swap.time = local_schedule.longest_timeline();
                        local_schedule.clear();
                        if (best_swaps.empty() or swap.time == best_swaps[0].time) best_swaps.push_back(swap);
                        else if (swap.time < best_swaps[0].time) {
                            best_swaps.clear();
                            best_swaps.push_back(swap);
                        }
                        tested_orders.insert(insertion_order);
                        swap.revert();
                        reset_job_times();
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

        void replace_if_better(const std::optional<task_order::deferred_swap>& best_swap) {
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
                  insertion_order(solution.insertion_order), tasks_count(solution.insertion_order.size()) {
            tested_orders.insert(insertion_order);
        }

        bool advance() {

            const std::optional<task_order::deferred_swap> best_swap = find_best_swap();
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
