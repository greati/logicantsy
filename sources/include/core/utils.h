#ifndef __CORE_UTILS__
#define __CORE_UTILS__

#include <array>
#include <vector>

namespace ltsy::utils {

    std::vector<int> tuple_from_position(int nvalues, int arity, int position);

    int position_from_tuple(int nvalues, int arity, const std::vector<int>& tuple);

};

#endif
