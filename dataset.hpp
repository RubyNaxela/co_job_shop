#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <fstream>
#include <vector>

namespace js {

    typedef uint16_t time16_t;
    typedef int16_t id16_t;

    struct task {

        struct job& parent;
        id16_t machine_id = 0;
        time16_t duration = 0, scheduled_time = 0;

        explicit task(job& parent) : parent(parent) {}
    };

    struct job {

        id16_t id;
        std::vector<task> sequence;
        time16_t last_scheduled_time = 0;

        explicit job(id16_t id) : id(id) {}
    };

    struct dataset {

        id16_t machine_count = 0;
        std::vector<job> jobs;

        void load_from_file(const char* path) {
            std::ifstream file(path);
            if (not file.is_open()) throw std::runtime_error("Could not open file " + std::string(path));
            id16_t job_count;
            file >> job_count;
            file >> machine_count;
            jobs.reserve(job_count);
            for (id16_t i = 0; i < job_count; i++) {
                job& t = jobs.emplace_back(i);
                t.sequence.reserve(machine_count);
                for (id16_t j = 0; j < machine_count; j++) {
                    task& st = t.sequence.emplace_back(t);
                    file >> st.machine_id;
                    file >> st.duration;
                }
            }
            file.close();
        }
    };
}

#endif //JOB_SHOP_DATASET
