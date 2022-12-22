#ifndef JOB_SHOP_LOCAL_SEARCH
#define JOB_SHOP_LOCAL_SEARCH

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"

#include "avl_tree_set.hpp"
#include "dataset.hpp"
#include "schedule.hpp"
#include "timer.hpp"

namespace js {

    class iteration_result {

        std::optional<task_order::deferred_swap> swap;
        bool eot;

        iteration_result() : swap(std::nullopt), eot(false) {}

        iteration_result(task_order::deferred_swap swap, bool time_over) : swap(swap), eot(time_over) {}

    public:

        static iteration_result found(const task_order::deferred_swap& swap, bool time_over = false) {
            return {swap, time_over};
        }

        static iteration_result not_found() {
            return {};
        }

        [[nodiscard]] bool success() const {
            return swap.has_value();
        }

        [[nodiscard]] bool time_over() const {
            return eot;
        }

        [[nodiscard]] task_order::deferred_swap get() const {
            return *swap;
        }
    };

    class local_search_environment {

        dataset data;
        solution& solution;
        task_order insertion_order;
        size_t tasks_count;
        zr::avl_tree_set<hash_t> tested_orders;
        const zr::stopwatch<zr::precision::us>& master_timer;

        bool insertion_order_tested(task_order::deferred_swap swap) {
            swap.commit();
            const bool tested = tested_orders.contains(insertion_order);
            swap.revert();
            return tested;
        }

        iteration_result find_best_swap() {
            std::vector<task_order::deferred_swap> best_swaps;
            schedule local_schedule(data);
            for (size_t t1 = 0; t1 < tasks_count - 1; t1++) {
                for (size_t t2 = t1 + 1; t2 < tasks_count; t2++) {
                    if (master_timer.is_over()) {
                        return not best_swaps.empty() ? iteration_result::found(best_swaps[best_swaps.size() / 2], true)
                                                      : iteration_result::not_found();
                    }
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
            return not best_swaps.empty() ? iteration_result::found(best_swaps[best_swaps.size() / 2])
                                          : iteration_result::not_found();
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

        explicit local_search_environment(js::solution& solution, const zr::stopwatch<zr::precision::us>& master_timer)
                : data(solution.data_source, solution.jobs_limit), solution(solution),
                  insertion_order(solution.insertion_order), tasks_count(solution.insertion_order.size()),
                  master_timer(master_timer) {
            tested_orders.insert(insertion_order);
        }

        bool advance() {

            const iteration_result result = find_best_swap();
            if (result.success()) replace_if_better(result.get());
            else return false;
            if (result.time_over()) return false;

            reset_job_times();
            schedule schedule(data);
            schedule.schedule_jobs(insertion_order);
            return true;
        }
    };

    void local_search(solution& solution, const zr::stopwatch<zr::precision::us>& master_timer) {
        local_search_environment environment(solution, master_timer);
        while (environment.advance());
    }
}

#pragma clang diagnostic pop

#endif //JOB_SHOP_LOCAL_SEARCH
