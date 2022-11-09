#ifndef JOB_SHOP_DISPATCHER
#define JOB_SHOP_DISPATCHER

#include <algorithm>
#include <numeric>
#include "dataset.hpp"
#include "heuristics.hpp"
#include "schedule.hpp"

namespace js {

    void schedule_tasks(js::dataset& data, js::schedule& schedule, const js::heuristic& heuristic) {
        const size_t sequence_length = data.tasks[0].sequence.size();
        for (size_t i = 0; i < sequence_length; i++) {
            std::vector < js::task * > tasks_order(data.tasks.size());
            std::iota(tasks_order.begin(), tasks_order.end(), &data.tasks[0]);
            std::sort(tasks_order.begin(), tasks_order.end(),
                      [=](const js::task* a, const js::task* b) { return heuristic(a, b, i); });
            for (const auto& task : tasks_order) schedule.add_sub_task(task->sequence[i]);
        }
    }
}

#endif //JOB_SHOP_DISPATCHER
