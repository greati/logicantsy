#ifndef __CORE_UTILS__
#define __CORE_UTILS__

#include <array>

namespace ltsy::utils {

    template<int NValues, int MArity>
    std::array<int, MArity> tuple_from_position(int position);

    template<int NValues, int MArity>
    int position_from_tuple(const std::array<int, MArity>& tuple);

};

#include "../../src/core/utils.cpp"
#endif
