#include <unordered_set>
#include "core/common.h"
#include <ostream>

std::ostream& operator<<(std::ostream& os, const std::unordered_set<int> &s)
{
    os << std::string("{");
	for (auto i = s.cbegin(); i != s.cend(); ++i) {
        os << *i;
        if (std::next(i) != s.cend())
            os << std::string(" ");
	}
    os << std::string("}");
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<int> &s)
{
    os << std::string("{");
	for (auto i = s.cbegin(); i != s.cend(); ++i) {
        os << *i;
        if (std::next(i) != s.cend())
            os << std::string(" ");
	}
    os << std::string("}");
	return os;
}
