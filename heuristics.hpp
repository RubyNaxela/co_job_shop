#ifndef JOB_SHOP_HEURISTICS
#define JOB_SHOP_HEURISTICS

#include <functional>
#include "dataset.hpp"

namespace js {

    typedef std::function<bool(const js::job*, const js::job*, size_t)> heuristic;

    namespace heuristics {

        bool alex_forward(const js::job* a, const js::job* b, size_t i) {
            return false;
        }

        bool alex_backward(const js::job* a, const js::job* b, size_t i) {
            if (a->sequence[i].duration == b->sequence[i].duration) return false;
            return true;
        }

        bool stachu_ascending(const js::job* a, const js::job* b, size_t i) {
            return a->sequence[i].duration < b->sequence[i].duration;
        }

        bool stachu_descending(const js::job* a, const js::job* b, size_t i) {
            return a->sequence[i].duration > b->sequence[i].duration;
        }

        heuristic get_by_name(const std::string& name) {
            if (name == "alex_forward") return alex_forward;
            if (name == "alex_backward") return alex_backward;
            if (name == "stachu_ascending") return stachu_ascending;
            if (name == "stachu_descending") return stachu_descending;
            throw std::invalid_argument("Unknown heuristic: " + name);
        }
    }
}

#endif //JOB_SHOP_HEURISTICS
