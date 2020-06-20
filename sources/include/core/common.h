#ifndef __COMMON__
#define __COMMON__

#include <string>
#include <unordered_set>
#include <set>

namespace ltsy {
    using Symbol = std::string;
    using Arity = int;
};

std::ostream& operator<<(std::ostream& os, const std::unordered_set<int> &s);
std::ostream& operator<<(std::ostream& os, const std::set<int> &s);

#endif
