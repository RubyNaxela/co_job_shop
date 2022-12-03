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

    struct task_order : public std::vector<js::task_coordinates> {

        explicit task_order(const std::vector<js::task_coordinates>& tasks) {
            std::vector<js::task_coordinates>::operator=(tasks);
        };

        hash_t hash() {
            hash_t hash = size();
            for (auto& coordinates : *this)
                hash ^= (uint64_t(coordinates.job_id) << 32 | coordinates.task_id)
                        + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }

        struct swap {

            size_t t1, t2;
            time32_t time = std::numeric_limits<time32_t>::max();

            swap(size_t t1, size_t t2) : t1(t1), t2(t2) {}
        };
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

        id32_t machines_count = 0, jobs_count = 0;
        std::vector<job> jobs;

        explicit dataset(const std::string& data_source, id32_t limit = 0) {
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
    };
}

#endif //JOB_SHOP_DATASET
