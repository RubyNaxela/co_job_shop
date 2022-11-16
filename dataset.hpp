#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <fstream>
#include <vector>

namespace js {

    typedef uint32_t time32_t;
    typedef int32_t id32_t;

    struct task {

        struct job& parent;
        id32_t machine_id = 0;
        time32_t duration = 0, scheduled_time = 0;

        explicit task(job& parent) : parent(parent) {}
    };

    struct job {

        id32_t id;
        std::vector<task> sequence;
        time32_t last_scheduled_time = 0;

        explicit job(id32_t id) : id(id) {}
    };

    struct dataset {

        id32_t machine_count = 0;
        std::vector<job> jobs;

        void load_from_file(const char* path) {
            std::ifstream file(path);
            if (not file.is_open()) throw std::runtime_error("Could not open file " + std::string(path));
            id32_t job_count;
            file >> job_count;
            file >> machine_count;
            jobs.reserve(job_count);
            for (id32_t i = 0; i < job_count; i++) {
                job& t = jobs.emplace_back(i);
                t.sequence.reserve(machine_count);
                for (id32_t j = 0; j < machine_count; j++) {
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
