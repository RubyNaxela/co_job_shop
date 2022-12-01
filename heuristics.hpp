#ifndef JOB_SHOP_HEURISTICS
#define JOB_SHOP_HEURISTICS

#include <functional>
#include <utility>
#include "dataset.hpp"

namespace js {

    struct heuristic {

        enum order_mod {
            do_nothing, reverse, sort
        };

        order_mod order_mod;
        std::function<bool(const job*, const job*, size_t)> comparator;
        std::string name;

        static bool pass(const job* a, const job* b, size_t i) {
            return false;
        }

        static bool ascending(const job* a, const job* b, size_t i) {
            return a->sequence[i].duration < b->sequence[i].duration;
        }

        static bool descending(const job* a, const job* b, size_t i) {
            return a->sequence[i].duration > b->sequence[i].duration;
        }
    };

    const heuristic heuristics[] = {
            {heuristic::do_nothing, heuristic::pass,       "alex_forward"},
            {heuristic::reverse,    heuristic::pass,       "alex_backward"},
            {heuristic::sort,       heuristic::ascending,  "ascending"},
            {heuristic::sort,       heuristic::descending, "descending"},
    };
}

#endif //JOB_SHOP_HEURISTICS
