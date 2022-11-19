#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <sstream>
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

        void load_from_memory(const std::string& data_string) {
            std::istringstream data_stream(data_string);
            id32_t job_count;
            data_stream >> job_count;
            data_stream >> machine_count;
            jobs.reserve(job_count);
            for (id32_t i = 0; i < job_count; i++) {
                job& t = jobs.emplace_back(i);
                t.sequence.reserve(machine_count);
                for (id32_t j = 0; j < machine_count; j++) {
                    task& st = t.sequence.emplace_back(t);
                    data_stream >> st.machine_id;
                    data_stream >> st.duration;
                }
            }
        }
    };
}

#endif //JOB_SHOP_DATASET
