#ifndef JOB_SHOP_HEURISTICS
#define JOB_SHOP_HEURISTICS

#include <functional>
#include <utility>
#include "dataset.hpp"

namespace js {

    enum heuristic_mod { do_nothing, reverse, sort };

    typedef std::pair<heuristic_mod, std::function<bool(const js::job*, const js::job*, size_t)>> heuristic;

    namespace heuristics {

        bool pass(const js::job* a, const js::job* b, size_t i) {
            return false;
        }

        bool stachu_ascending(const js::job* a, const js::job* b, size_t i) {
            return a->sequence[i].duration < b->sequence[i].duration;
        }

        bool stachu_descending(const js::job* a, const js::job* b, size_t i) {
            return a->sequence[i].duration > b->sequence[i].duration;
        }
    }
}

#endif //JOB_SHOP_HEURISTICS
