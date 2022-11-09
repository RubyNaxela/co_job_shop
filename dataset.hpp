#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <fstream>
#include <vector>

namespace js {

    struct sub_task {

        struct task& parent;
        int16_t machine_id = 0;
        size_t duration = 0, scheduled_time = 0;

        explicit sub_task(task& parent) : parent(parent) {}
    };

    struct task {

        int16_t id;
        std::vector<sub_task> sequence;
        size_t last_scheduled_time = 0;

        explicit task(int16_t id) : id(id) {}
    };

    struct dataset {

        int16_t machine_count = 0;
        std::vector<task> tasks;

        void load_from_file(const char* path) {
            std::ifstream file(path);
            if (not file.is_open()) throw std::runtime_error("Could not open file " + std::string(path));
            int16_t task_count;
            file >> task_count;
            file >> machine_count;
            tasks.reserve(task_count);
            for (int16_t i = 0; i < task_count; i++) {
                task& t = tasks.emplace_back(i);
                t.sequence.reserve(machine_count);
                for (int16_t j = 0; j < machine_count; j++) {
                    sub_task& st = t.sequence.emplace_back(t);
                    file >> st.machine_id;
                    file >> st.duration;
                }
            }
            file.close();
        }
    };
}

#endif //JOB_SHOP_DATASET
