#ifndef JOB_SHOP_SCHEDULE
#define JOB_SHOP_SCHEDULE

#include <vector>
#include "dataset.hpp"

namespace js {

    struct schedule {

        std::vector<std::vector<size_t>> table;

        void add_sub_task(sub_task t);
    };
}

#endif //JOB_SHOP_SCHEDULE
