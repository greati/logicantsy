#pragma once

#include <chrono>
#include <iostream>

namespace progresscpp {
class ProgressBar {
private:
    unsigned long long int ticks = 0;

    unsigned long long int total_ticks = 0;
    const unsigned int bar_width;
    const char complete_char = '=';
    const char incomplete_char = ' ';
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

public:
    ProgressBar(unsigned long long total, unsigned int width, char complete, char incomplete) :
            total_ticks{total}, bar_width{width}, complete_char{complete}, incomplete_char{incomplete} {}

    ProgressBar(unsigned long long total, unsigned int width) : total_ticks{total}, bar_width{width} {}
    ProgressBar(unsigned int width) : bar_width{width} {}

    unsigned long long int operator++() { return ++ticks; }

    void set_total_ticks(decltype(total_ticks) _total_ticks) {
        total_ticks = _total_ticks;
    }

    void display() const {
        float progress = (float) ticks / total_ticks;
        int pos = (int) (bar_width * progress);

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

        std::cout << "[";

        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) std::cout << complete_char;
            else if (i == pos) std::cout << ">";
            else std::cout << incomplete_char;
        }
        std::cout << "] " << int(progress * 100.0) << "% "
                  << float(time_elapsed) / 1000.0 << "s\r";
        std::cout.flush();
    }

    void done() const {
        display();
        std::cout << std::endl;
    }
};
}
