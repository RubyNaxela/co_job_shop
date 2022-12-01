#ifndef JOB_SHOP_DATASET
#define JOB_SHOP_DATASET

#include <sstream>
#include <vector>

namespace js {

    typedef uint32_t time32_t;
    typedef int32_t id32_t;
    typedef uint64_t hash_t;

    struct job {

        id32_t id;
        std::vector<struct task> sequence;
        time32_t last_scheduled_time = 0;

        explicit job(id32_t id) : id(id) {}
    };

    struct task_coordinates {
        id32_t job_id, task_id;
    };

    struct task {

        job& parent;
        id32_t machine_id = 0, sequence_order = 0;
        time32_t duration = 0, scheduled_time = 0;

        explicit task(job& parent) : parent(parent) {}

        [[nodiscard]] task_coordinates coordinates() const {
            return {parent.id, sequence_order};
        }
    };

    struct dataset {

        size_t machines_count = 0, jobs_count = 0;
        std::vector<job> jobs;

        explicit dataset(const std::string& data_source, uint16_t limit = 0) {
            std::istringstream data_stream(data_source);
            data_stream >> jobs_count;
            data_stream >> machines_count;
            if (limit > 0) jobs_count = limit;
            jobs.reserve(jobs_count);
            for (id32_t j = 0; j < jobs_count; j++) {
                job& job = jobs.emplace_back(j);
                job.sequence.reserve(machines_count);
                for (id32_t m = 0; m < machines_count; m++) {
                    task& task = job.sequence.emplace_back(job);
                    task.sequence_order = m;
                    data_stream >> task.machine_id;
                    data_stream >> task.duration;
                }
            }
        }

        [[nodiscard]] task& get_task(task_coordinates coordinates) const {
            return const_cast<task&>(std::find_if(jobs.begin(), jobs.end(), [&](const job& job) {
                return job.id == coordinates.job_id;
            })->sequence[coordinates.task_id]);
        }

        void reset_schedule_times() {
            for (job& job : jobs) job.last_scheduled_time = 0;
        }
    };
}

#endif //JOB_SHOP_DATASET
