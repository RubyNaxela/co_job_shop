#ifndef JOB_SHOP_BASIC_SEARCH
#define JOB_SHOP_BASIC_SEARCH

#include "heuristics.hpp"
#include "schedule.hpp"
#include "solution.hpp"

namespace js {

    solution find_initial_solution(const std::string& data_source, uint16_t jobs_limit, bool gantt_chart) {

        solution solution;

        for (const auto& heuristic : heuristics) {

            schedule schedule(dataset(data_source, jobs_limit));
            std::vector<task*> insertion_order = schedule.schedule_jobs(heuristic);

            if (not solution.time.has_value() or schedule.longest_timeline() < solution.time) {
                if (gantt_chart) solution.schedule = *dynamic_cast<basic_schedule*>(&schedule);
                solution.insertion_order.clear();
                std::transform(insertion_order.begin(), insertion_order.end(), std::back_inserter(solution.insertion_order),
                               [](const task* t) { return t->coordinates(); });
                solution.summary = schedule.summary();
                solution.time = schedule.longest_timeline();
            }
        }

        return solution;
    }
}

#endif //JOB_SHOP_BASIC_SEARCH
