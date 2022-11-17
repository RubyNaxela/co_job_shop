#ifndef JOB_SHOP_SCHEDULE
#define JOB_SHOP_SCHEDULE

#include <ostream>
#include <sstream>
#include <vector>
#include "dataset.hpp"
#include "platform.hpp"

namespace js {

    class schedule {

        std::vector<std::vector<interval*>> table;
        dataset& data;

        static void schedule_task(std::vector<interval*>& timeline, interval* interval, task& task, time32_t start) {

            const bool empty_before = start > interval->start, empty_after = start + task.duration - 1 < interval->end;
            const time32_t interval_start = interval->start, interval_end = interval->end;

            interval->task = &task;
            interval->start = start;
            interval->end = start + task.duration - 1;

            if (empty_before) {
                timeline.push_back(interval::empty(interval_start, start - 1));
                std::swap(timeline[timeline.size() - 1], timeline[timeline.size() - 2]);
            }
            if (empty_after) {
                timeline.push_back(interval::empty(start + task.duration, interval_end));
            }

            task.set_scheduled_time(start, task.parent.last_scheduled_time = start + task.duration);
        }

        [[nodiscard]] time32_t longest_timeline() const {
            time32_t result = 0;
            for (const auto& timeline : table)
                if (timeline.back()->occupied()) result = std::max(result, timeline.back()->end);
                else result = std::max(result, timeline.back()->start);
            return result;
        }

        static std::vector<id32_t> get_timeline(const std::vector<interval*>& timeline, time32_t length) {
            std::vector<id32_t> result;
            for (const auto& interval : timeline) {
                for (time32_t i = interval->start; i <= interval->end and i < length; i++) {
                    if (interval->occupied()) result.push_back(interval->task->parent.id);
                    else result.push_back(-1);
                }
            }
            return result;
        }

#ifndef WINDOZE

        static std::string colored(const char* text, id32_t color) {
            return "\033[1;" + std::to_string(31 + color % 6) + "m" + text + "\033[0m";
        }

#else
        static std::string colored(const char* text, id32_t color) {
            return text;
        }
#endif

    public:

        explicit schedule(dataset& data) : table(std::vector<std::vector<interval*>>(data.machine_count)), data(data) {
            for (auto& timeline : table) timeline.push_back(interval::empty());
        }

        void add_task(task& task) {
            std::vector<interval*>& timeline = table[task.machine_id];
            const time32_t job_end = task.parent.last_scheduled_time;
            size_t i_index = std::find_if(timeline.begin(), timeline.end(),
                                          [=](const interval* i) { return i->includes(job_end); }) - timeline.begin();
            interval* time = timeline[i_index];
            while (time->occupied() or not time->includes(job_end, task.duration)) time = timeline[++i_index];
            schedule_task(timeline, time, task, std::max(time->start, job_end));
        }

        [[nodiscard]] std::string gantt_chart() {

            std::stringstream chart;

            const time32_t longest = longest_timeline();
            const auto cell_width = uint8_t(std::max(log10(longest), log10(data.jobs.size())) + 1);
            const auto left_col_width = uint8_t(log10(data.machine_count) + 1);
            const std::string l_hd_format = "%0" + std::to_string(left_col_width) + "hd";
            const std::string zu_format = "%0" + std::to_string(cell_width) + "zu";
            const std::string hd_format = "%0" + std::to_string(cell_width) + "hd";
            const char* fmt1 = l_hd_format.c_str(), * fmt2 = zu_format.c_str(), * fmt3 = hd_format.c_str();
            const std::string empty = std::string(cell_width, '_') + '|';

            chart << "   " + std::string(left_col_width, ' ');
            char* id_string = new char[3];
            for (time32_t i = 0; i < longest; i++) {
                sprintf(id_string, fmt2, i);
                chart << id_string << " ";
            }
            chart << std::endl;

            for (id32_t machine_id = 0; machine_id < (id32_t) table.size(); machine_id++) {
                sprintf(id_string, fmt1, machine_id);
                chart << id_string << ": ";
                chart << '|';
                for (id32_t task_id : get_timeline(table[machine_id], longest)) {
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
                for (const auto& sub_task : task.sequence) summary << sub_task.scheduled_time.start << ' ';
                summary << '\n';
            }
            return summary.str();
        };
    };
}

#endif //JOB_SHOP_SCHEDULE
