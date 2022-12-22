#ifndef JOB_SHOP_SOLUTION
#define JOB_SHOP_SOLUTION

#include <optional>
#include <string>
#include <vector>
#include "dataset.hpp"
#include "schedule.hpp"

namespace js {

    struct solution {

        const std::string& data_source;
        const uint16_t jobs_limit;

        js::basic_schedule schedule;
        std::vector<js::task_coordinates> insertion_order;
        std::string summary;
        std::optional<js::time32_t> time;

        solution(const std::string& data_source, uint16_t jobs_limit)
                : data_source(data_source), jobs_limit(jobs_limit) {}

        std::string gantt_chart() {
            return schedule.gantt_chart();
        }
    };
}

#endif //JOB_SHOP_SOLUTION
