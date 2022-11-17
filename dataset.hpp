#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <fstream>
#include <vector>

namespace js {

    typedef uint32_t time32_t;
    typedef int32_t id32_t;

    struct interval {

        time32_t start, end;
        struct task* task;

        inline static time32_t infinity = std::numeric_limits<time32_t>::max();

        interval(time32_t start, time32_t end, struct task* task) : start(start), end(end), task(task) {}

        static interval empty(time32_t start = 0, time32_t end = infinity) {
            return {start, end, nullptr};
        }

        [[nodiscard]] bool occupied() const {
            return task != nullptr;
        }

        [[nodiscard]] bool includes(time32_t time) const {
            return start <= time and time <= end;
        }

        [[nodiscard]] bool includes(time32_t from, time32_t duration) const {
            return std::max(start, from) + duration - 1 <= end;
        }
    };

    struct task {

        struct job& parent;
        id32_t machine_id = 0;
        time32_t duration = 0;
        interval* scheduled_time = nullptr;

        explicit task(job& parent) : parent(parent) {}

        void set_scheduled_time(interval* time) {
            scheduled_time = time;
            scheduled_time->task = this;
        }
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
