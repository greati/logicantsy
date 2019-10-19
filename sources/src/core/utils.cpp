#include "core/utils.h"
#include "core/exception.h"
#include <iostream>
#include <cmath>

namespace ltsy::utils {

    template<int NValues, int MArity>
    std::array<int, MArity> tuple_from_position(int position) {
        if (position < 0 or position > int(std::pow(NValues, MArity)))
            throw std::invalid_argument(ltsy::WRONG_TUPLE_POSITION_EXCEPTION);
        auto i = MArity - 1;
        std::array<int, MArity> tuple;
        while (i >= 0) {
            auto power = int(std::pow(NValues, i));
            tuple[MArity - i - 1] = position / power;
            position %= power;
            i -= 1;
        }
        return tuple;
    }

    template<int NValues, int MArity>
    int position_from_tuple(const std::array<int, MArity>& tuple) {
        int i = 0;
        int position = 0;
        while (i < MArity) {
            auto power = int(std::pow(NValues, MArity - i - 1));
            position += tuple[i] * power;
            ++i;
        }
        return position;
    }
};
