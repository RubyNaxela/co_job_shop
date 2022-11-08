#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <fstream>
#include <vector>

namespace js {

    struct task;

    struct sub_task {
        int16_t machine_id = -1, task_id = -1;
        size_t duration = 0, scheduled_time = -1;
    };

    struct task {
        int16_t id = -1;
        std::vector<sub_task> sequence;
        size_t last_scheduled_time = 0;
    };

    struct dataset {
        size_t machine_count = 0;
        std::vector<task> tasks;

        static dataset from_file(const char* path) {
            std::ifstream file(path);
            if (not file.is_open()) throw std::runtime_error("Could not open file " + std::string(path));
            dataset result;
            size_t task_count;
            file >> task_count;
            file >> result.machine_count;
            for (size_t i = 0; i < task_count; i++) {
                task t;
                t.id = (int16_t) i;
                for (size_t j = 0; j < result.machine_count; j++) {
                    sub_task st;
                    file >> st.machine_id;
                    file >> st.duration;
                    st.task_id = (int16_t) i;
                    t.sequence.push_back(st);
                }
                result.tasks.push_back(t);
            }
            return result;
        }

        task& get_task(int16_t id) {
            return *std::find_if(tasks.begin(), tasks.end(),
                                 [&](const auto& item) { return item.id == id; });
        }
    };
}

#endif //JOB_SHOP_DATASET
