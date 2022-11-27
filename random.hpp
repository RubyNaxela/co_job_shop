#ifndef JOB_SHOP_RANDOM
#define JOB_SHOP_RANDOM

#include <random>
#include "range.hpp"

namespace js {

    template<typename Tp>
    requires std::is_integral_v<Tp>
    Tp random_number(Tp from, Tp to) {
        std::random_device rd;
        std::uniform_int_distribution<Tp> dis(from, to);
        return dis(rd);
    }

    template<typename Tp>
    requires std::is_integral_v<Tp>
    Tp random_number(const range<Tp>& range) {
        std::random_device rd;
        std::uniform_int_distribution<Tp> dis(range.left, range.right);
        return dis(rd);
    }
}

#endif //JOB_SHOP_RANDOM
