#ifndef JOB_SHOP_RANGE
#define JOB_SHOP_RANGE

#include <limits>

namespace zr {

    enum range_class {
        empty,
        singleton,
        open,
        closed,
        open_closed,
        closed_open,
        open_infinite,
        closed_infinite,
        infinite_open,
        infinite_closed,
        infinite
    };

    template<typename Tp> requires std::is_integral_v<Tp>
    struct range {

        Tp left, right;

        range(Tp left, Tp right) : left(left), right(right) {}

        template<typename OTp>
        requires std::is_integral_v<OTp>
        [[nodiscard]] bool includes(OTp value) const {
            return left <= value and value <= right;
        }

        template<typename OTp>
        requires std::is_integral_v<OTp>
        [[nodiscard]] bool includes(const range<OTp>& range) const {
            return left <= range.left and range.right <= right;
        }
    };

    template<typename Tp, int Class>
    requires (Class == empty)
    range<Tp> make_range() {
        return {std::numeric_limits<Tp>::max(), std::numeric_limits<Tp>::min()};
    }

    template<typename Tp, int Class>
    requires (Class == singleton)
    range<Tp> make_range(Tp value) {
        return {value, value};
    }

    template<typename Tp, int Class>
    requires (Class == open or Class == closed or Class == open_closed or Class == closed_open)
    range<Tp> make_range(Tp&& left, Tp&& right) {
        if constexpr (Class == open) return {left + 1, right - 1};
        else if constexpr (Class == closed) return {left, right};
        else if constexpr (Class == open_closed) return {left + 1, right};
        else if constexpr (Class == closed_open) return {left, right - 1};
    }

    template<typename Tp, int Class>
    requires (Class == open_infinite or Class == closed_infinite or Class == infinite_open or Class == infinite_closed)
    range<Tp> make_range(Tp&& endpoint) {
        if constexpr (Class == open_infinite) return {endpoint + 1, std::numeric_limits<Tp>::max()};
        else if constexpr (Class == closed_infinite) return {endpoint, std::numeric_limits<Tp>::max()};
        else if constexpr (Class == infinite_open) return {std::numeric_limits<Tp>::min(), endpoint - 1};
        else if constexpr (Class == infinite_closed) return {std::numeric_limits<Tp>::min(), endpoint};
    }

    template<typename Tp, int Class>
    requires (Class == infinite)
    range<Tp> make_range() {
        return {std::numeric_limits<Tp>::min(), std::numeric_limits<Tp>::max()};
    }
}

#endif //JOB_SHOP_RANGE
