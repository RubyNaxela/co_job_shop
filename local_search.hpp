#ifndef JOB_SHOP_LOCAL_SEARCH
#define JOB_SHOP_LOCAL_SEARCH

#include "avl_tree_set.hpp"
#include "dataset.hpp"
#include "schedule.hpp"
#include "timer.hpp"

namespace js {

    struct swap {
        size_t t1 = 0, t2 = 0;
        time32_t time = interval::infinity;
    };

    hash_t hash_insertion_order(const std::vector<task_coordinates>& insertion_order) {
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

    bool insertion_order_tested(std::vector<task_coordinates> insertion_order, const zr::avl_tree_set<hash_t>& tested_orders,
                                size_t t1, size_t t2) {
        std::swap(insertion_order[t1], insertion_order[t2]);
        return tested_orders.contains(hash_insertion_order(insertion_order));
    }

    void local_search(const std::string& data_string, uint16_t limit, auto& solution,
                      zr::timer<zr::precision::ms>& master_timer, uint64_t time_limit) {

        dataset data(data_string, limit);
        std::vector<task_coordinates> insertion_order = solution.insertion_order;
        zr::avl_tree_set<hash_t> tested_orders;
        tested_orders.insert(hash_insertion_order(insertion_order));

        const uint64_t start_time = master_timer.get_measured_time();
        size_t iterations = 0;
        const auto average_iteration_time = [&]() { return (master_timer.get_measured_time() - start_time) / iterations; };

        do {
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
                        schedule local_schedule(data);
                        local_schedule.schedule_jobs(insertion_order);
                        swap.time = local_schedule.longest_timeline();
                        if (best_swaps.empty() or swap.time < best_swaps[0].time) {
                            best_swaps.clear();
                            best_swaps.push_back(swap);
                        } else if (best_swaps[0].time == swap.time)
                            best_swaps.push_back(swap);
                        tested_orders.insert(hash_insertion_order(insertion_order));

                        std::swap(insertion_order[i], insertion_order[j]);
                        for (job& job : data.jobs) job.last_scheduled_time = 0;
                    }
                }
            }
            std::optional<swap> best_swap = not best_swaps.empty()
                                            ? std::make_optional(best_swaps[best_swaps.size() / 2])
                                            : std::nullopt;
            if (best_swap.has_value()) {
                std::swap(insertion_order[best_swap.value().t1], insertion_order[best_swap.value().t2]);
                tested_orders.insert(hash_insertion_order(insertion_order));
                schedule local_schedule(data);
                local_schedule.schedule_jobs(insertion_order);
                if (local_schedule.longest_timeline() < solution.time) {
                    solution.schedule = *dynamic_cast<basic_schedule*>(&local_schedule);
                    solution.insertion_order = insertion_order;
                    solution.summary = local_schedule.summary();
                    solution.time = local_schedule.longest_timeline();
                }
            } else break;

            for (job& job : data.jobs) job.last_scheduled_time = 0;

            schedule schedule(data);
            schedule.schedule_jobs(insertion_order);

            iterations++;

        } while (master_timer.get_measured_time() < time_limit - average_iteration_time());
    }
}

#endif //JOB_SHOP_LOCAL_SEARCH
