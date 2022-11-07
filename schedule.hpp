#ifndef JOB_SHOP_SCHEDULE
#define JOB_SHOP_SCHEDULE

#include <ostream>
#include <vector>
#include "dataset.hpp"

namespace js {

    class schedule {

        std::vector<std::vector<int16_t>> table;

        [[nodiscard]] int16_t get_cell(int16_t machine_id, size_t time_unit) const {
            if (time_unit >= table[machine_id].size()) return -1;
            return table[machine_id][time_unit];
        }

        bool is_available(const sub_task& task, size_t start) {
            for (int16_t m = 0; m < (int16_t) table.size(); m++)
                for (size_t t = start; t < start + task.duration; t++) {
                    if (get_cell(task.machine_id, t) != -1 or get_cell(m, t) == task.task_id) return false;
                }
            return true;
        }

        static void schedule_sub_task(std::vector<int16_t>& timeline, sub_task& task, size_t start) {
            const size_t end = start + task.duration;
            if (end >= timeline.size()) timeline.resize(end, -1);
            for (size_t t = start; t < end; t++) timeline[t] = task.task_id;
//            std::cout << "scheduled subtask " << task.task_id << "(" << task.machine_id << ")<" << task.duration << ">"
//                      << " @ " << start << " ~ " << end << std::endl;
        }

        static std::string colored(const char* text, int16_t color) {
            return "\033[1;" + std::to_string(31 + color % 6) + "m" + text + "\033[0m";
        }

    public:

        explicit schedule(size_t machine_count) : table(std::vector<std::vector<int16_t>>(machine_count)) {}

        void add_sub_task(sub_task& task) {
            std::vector<int16_t>& timeline = table[task.machine_id];
            size_t t = 0;
            while (not is_available(task, t)) t++;
            schedule_sub_task(timeline, task, t);
        }

        friend std::ostream& operator<<(std::ostream& os, const schedule& s) {
            char* id_string = new char[3];
            os << "     ";
            for (uint8_t i = 0; i < 64; i++) {
                sprintf(id_string, "%02hd", i);
                os << id_string << " ";
            }
            os << std::endl;
            for (int16_t machine_id = 0; machine_id < (int16_t) s.table.size(); machine_id++) {
                sprintf(id_string, "%02hd", machine_id);
                os << id_string << ": ";
                os << '|';
                for (int16_t task_id : s.table[machine_id]) {
                    sprintf(id_string, "%02hd", task_id);
                    if (task_id == -1) os << "__|";
                    else os << colored(id_string, task_id) << '|';
                }
                os << std::endl;
            }
            delete[] id_string;
            return os;
        };
    };
}

#endif //JOB_SHOP_SCHEDULE
