#ifndef JOB_SHOP_SCHEDULE
#define JOB_SHOP_SCHEDULE

#include <list>
#include <ostream>
#include <sstream>
#include <vector>
#include "dataset.hpp"
#include "platform.hpp"

namespace js {

    class timeline {

        std::list<interval> intervals;

    public:

        typedef std::list<interval>::iterator pointer;

        [[nodiscard]] pointer interval_at(time32_t time) {
            return std::find_if(intervals.begin(), intervals.end(),
                                [=](const interval i) { return i.includes(time); });
        }

        [[nodiscard]] time32_t length() const {
            const interval& last = intervals.back();
            if (last.occupied()) return last.end + 1;
            else return last.start;
        }

        void add(interval&& interval) {
            intervals.push_back(interval);
        }

        void insert_before(const pointer& it, interval&& interval) {
            intervals.insert(it, interval);
        }

        void insert_after(const pointer& it, interval&& interval) {
            intervals.insert(std::next(it), interval);
        }

        [[nodiscard]] std::vector<id32_t> quantized(time32_t limit) const {
            std::vector<id32_t> result;
            for (const auto& interval : intervals) {
                for (time32_t i = interval.start; i <= interval.end and i < limit; i++) {
                    if (interval.occupied()) result.push_back(interval.task->parent.id);
                    else result.push_back(-1);
                }
            }
            return result;
        }

        bool operator<(const timeline& other) const {
            return length() < other.length();
        }
    };

    class schedule {

        std::vector<timeline> table;
        dataset& data;

        static void schedule_task(task& task, timeline& timeline, const timeline::pointer& interval, time32_t start) {

            const time32_t interval_start = interval->start, interval_end = interval->end;
            const bool empty_before = start > interval_start, empty_after = start + task.duration - 1 < interval_end;

            interval->task = &task;
            interval->start = start;
            interval->end = start + task.duration - 1;

            if (empty_before) timeline.insert_before(interval, interval::empty(interval_start, start - 1));
            if (empty_after) timeline.insert_after(interval, interval::empty(start + task.duration, interval_end));

            task.scheduled_time = start;
            task.parent.last_scheduled_time = start + task.duration;
        }

#ifndef WINDOZE

        static std::string

        colored(const char* text, id32_t color) {
            return "\033[1;" + std::to_string(31 + color % 6) + "m" + text + "\033[0m";
        }

#else
        static std::string colored(const char* text, id32_t color) {
            return text;
        }
#endif

    public:

        [[nodiscard]] time32_t longest_timeline() const {
            return std::max_element(table.begin(), table.end())->length();
        }

        explicit schedule(dataset& data) : table(std::vector<timeline>(data.machine_count)), data(data) {
            for (auto& timeline : table) timeline.add(interval::empty());
        }

        void add_task(task& task) {
            timeline& timeline = table[task.machine_id];
            const time32_t job_end = task.parent.last_scheduled_time;
            auto time = timeline.interval_at(job_end);
            while (time->occupied() or not time->includes(job_end, task.duration)) ++time;
            schedule_task(task, timeline, time, std::max(time->start, job_end));
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
                for (id32_t task_id : table[machine_id].quantized(longest)) {
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
