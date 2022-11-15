#ifndef JOB_SHOP_SCHEDULE
#define JOB_SHOP_SCHEDULE

#include <ostream>
#include <sstream>
#include <vector>
#include "dataset.hpp"
#include "platform.hpp"

namespace js {

    class schedule {

        std::vector<std::vector<id16_t>> table;
        dataset& data;

        [[nodiscard]] id16_t get_cell(id16_t machine_id, time16_t time_unit) const {
            if (time_unit >= table[machine_id].size()) return -1;
            return table[machine_id][time_unit];
        }

        bool is_available(const task& task, time16_t start) {
            for (id16_t m = 0; m < (id16_t) table.size(); m++)
                for (time16_t t = start; t < start + task.duration; t++) {
                    if (get_cell(task.machine_id, t) != -1) return false;
                }
            return true;
        }

        static void schedule_task(std::vector<id16_t>& timeline, task& task, time16_t start) {
            const time16_t end = start + task.duration;
            if (end >= timeline.size()) timeline.resize(end, -1);
            for (time16_t t = start; t < end; t++) timeline[t] = task.parent.id;
            task.scheduled_time = start;
            task.parent.last_scheduled_time = start + task.duration;
        }

        [[nodiscard]] time16_t longest_timeline() const {
            time16_t result = 0;
            for (const auto& timeline : table) result = std::max(result, time16_t(timeline.size()));
            return result;
        }

#ifndef WINDOZE

        static std::string colored(const char* text, id16_t color) {
            return "\033[1;" + std::to_string(31 + color % 6) + "m" + text + "\033[0m";
        }

#else
        static std::string colored(const char* text, id16_t color) {
            return text;
        }
#endif

    public:

        explicit schedule(dataset& data)
                : table(std::vector<std::vector<id16_t>>(data.machine_count)), data(data) {}

        void add_task(task& task) {
            std::vector<id16_t>& timeline = table[task.machine_id];
            time16_t t = task.parent.last_scheduled_time;
            while (not is_available(task, t)) t++;
            schedule_task(timeline, task, t);
        }

        [[nodiscard]] std::string gantt_chart() {

            std::stringstream chart;

            const time16_t longest = longest_timeline();
            const auto cell_width = uint8_t(std::max(log10(longest), log10(data.jobs.size())) + 1);
            const auto left_col_width = uint8_t(log10(data.machine_count) + 1);
            const std::string l_hd_format = "%0" + std::to_string(left_col_width) + "hd";
            const std::string zu_format = "%0" + std::to_string(cell_width) + "zu";
            const std::string hd_format = "%0" + std::to_string(cell_width) + "hd";
            const char* fmt1 = l_hd_format.c_str(), * fmt2 = zu_format.c_str(), * fmt3 = hd_format.c_str();
            const std::string empty = std::string(cell_width, '_') + '|';

            chart << "   " + std::string(left_col_width, ' ');
            char* id_string = new char[3];
            for (time16_t i = 0; i < longest; i++) {
                sprintf(id_string, fmt2, i);
                chart << id_string << " ";
            }
            chart << std::endl;

            for (id16_t machine_id = 0; machine_id < (id16_t) table.size(); machine_id++) {
                sprintf(id_string, fmt1, machine_id);
                chart << id_string << ": ";
                chart << '|';
                for (id16_t task_id : table[machine_id]) {
                    sprintf(id_string, fmt3, task_id);
                    if (task_id == -1) chart << empty;
                    else chart << colored(id_string, task_id) << '|';
                }
                chart << std::endl;
            }

            delete[] id_string;
            return chart.str();
        };

        [[nodiscard]] std::string summary() const {
            std::stringstream summary;
            summary << longest_timeline() << '\n';
            std::vector<job>& tasks = data.jobs;
            std::sort(tasks.begin(), tasks.end(), [](const job& a, const job& b) { return a.id < b.id; });
            for (const auto& task : tasks) {
                for (const auto& sub_task : task.sequence) summary << sub_task.scheduled_time << ' ';
                summary << '\n';
            }
            return summary.str();
        };
    };
}

#endif //JOB_SHOP_SCHEDULE
