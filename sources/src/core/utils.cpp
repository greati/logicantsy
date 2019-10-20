#include "core/utils.h"
#include "core/exception.h"
#include <iostream>
#include <cmath>

namespace ltsy::utils {

    std::vector<int> tuple_from_position(int nvalues, int arity, int position) {
        if (position < 0 or position > int(std::pow(nvalues, arity)))
            throw std::invalid_argument(ltsy::WRONG_TUPLE_POSITION_EXCEPTION);
        auto i = arity - 1;
        std::vector<int> tuple (arity, 0);
        while (i >= 0) {
            auto power = int(std::pow(nvalues, i));
            tuple[arity - i - 1] = position / power;
            position %= power;
            i -= 1;
        }
        return tuple;
    }

    int position_from_tuple(int nvalues, int arity, const std::vector<int>& tuple) {
        int i = 0;
        int position = 0;
        while (i < arity) {
            auto power = int(std::pow(nvalues, arity - i - 1));
            position += tuple[i] * power;
            ++i;
        }
        return position;
    }
};
