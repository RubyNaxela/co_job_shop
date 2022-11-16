#ifndef JOB_SHOP_DISPATCHER
#define JOB_SHOP_DISPATCHER

#include <algorithm>
#include <numeric>
#include "dataset.hpp"
#include "heuristics.hpp"
#include "schedule.hpp"

namespace js {

    void schedule_jobs(js::dataset& data, js::schedule& schedule, const js::heuristic& heuristic) {
        const time32_t sequence_length = data.jobs[0].sequence.size();
        for (time32_t i = 0; i < sequence_length; i++) {
            std::vector<js::job*> tasks_order(data.jobs.size());
            std::iota(tasks_order.begin(), tasks_order.end(), &data.jobs[0]);
            std::sort(tasks_order.begin(), tasks_order.end(),
                      [=](const js::job* a, const js::job* b) { return heuristic(a, b, i); });
            for (job* task : tasks_order) schedule.add_task(task->sequence[i]);
        }
    }
}

#endif //JOB_SHOP_DISPATCHER
