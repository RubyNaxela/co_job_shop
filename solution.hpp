#ifndef JOB_SHOP_SOLUTION
#define JOB_SHOP_SOLUTION

#include <optional>
#include <string>
#include <vector>
#include "dataset.hpp"
#include "schedule.hpp"

namespace js {

    struct solution {

        js::basic_schedule schedule;
        std::vector<js::task_coordinates> insertion_order;
        std::string summary;
        std::optional<js::time32_t> time;

        std::string&& gantt_chart() {
            return std::move(schedule.gantt_chart());
        }
    };
}

#endif //JOB_SHOP_SOLUTION
