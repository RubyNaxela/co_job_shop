#ifndef JOB_SHOP_TIMER
#define JOB_SHOP_TIMER

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace zr {

    template<typename Dur> requires std::chrono::__is_duration<Dur>::value
    class timer {

        std::chrono::time_point<std::chrono::high_resolution_clock, Dur> start_point, end_point;
        bool running = false;

    public:

        timer() {
            start_point = std::chrono::high_resolution_clock::now();
            end_point = std::chrono::high_resolution_clock::now();
            running = false;
        }

        std::chrono::time_point<std::chrono::high_resolution_clock, Dur> now() const {
            return std::chrono::high_resolution_clock::now();
        }

        void start() {
            if (!running) {
                running = true;
                start_point = now();
            } else throw std::runtime_error("Illegal timer state: timer is already running");
        }

        void stop() {
            if (running) {
                end_point = now();
                running = false;
            } else throw std::runtime_error("Illegal timer state: timer is not running");
        }

        uint64_t get_measured_time() {
            end_point = now();
            Dur time = std::chrono::duration_cast<Dur>(end_point - start_point);
            return time.count();
        }

        std::string unit() {
            constexpr intmax_t denominator = Dur::period::den;
            if (denominator == 1) return "s";
            else if (denominator == 1000) return "ms";
            else if (denominator == 1000000) return "us";
            else if (denominator == 1000000000) return "ns";
            return "?";
        }
    };

    namespace precision {
        typedef std::chrono::duration<double, std::nano> ns;
        typedef std::chrono::duration<double, std::micro> us;
        typedef std::chrono::duration<double, std::milli> ms;
        typedef std::chrono::duration<double, std::ratio<1, 1>> s;
    }
}

#endif //JOB_SHOP_TIMER
