#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <fstream>
#include <vector>

namespace js {

    struct sub_task {
        size_t machine_id = -1;
        size_t duration = 0;
    };

    struct task {
        std::vector<sub_task> sequence;
    };

    struct dataset {
        size_t machine_count = 0;
        std::vector<task> tasks;

        static dataset from_file(const char* path) {
            std::ifstream file(path);
            dataset result;
            size_t task_count;
            file >> task_count;
            file >> result.machine_count;
            for (size_t i = 0; i < task_count; i++) {
                task t;
                for (size_t j = 0; j < result.machine_count; j++) {
                    sub_task st;
                    file >> st.machine_id;
                    file >> st.duration;
                    t.sequence.push_back(st);
                }
                result.tasks.push_back(t);
            }
            return result;
        }
    };
}

#endif //JOB_SHOP_DATASET
