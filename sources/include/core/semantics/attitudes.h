#ifndef __ATTITUDES__
#define __ATTITUDES__

#include <set>
#include "core/common.h"
#include <iostream>

namespace ltsy {
    struct CognitiveAttitude {
        Symbol symbol;
        std::set<int> values;

        CognitiveAttitude(const Symbol& symbol, const decltype(values)& values) : values {values}, symbol {symbol} {/*empty*/}

        bool operator<(const CognitiveAttitude& other) const {
            return this->symbol < other.symbol;
        }
        bool operator==(const CognitiveAttitude& other) const {
            return this->symbol == other.symbol;
        }
    };
}

std::ostream& operator<<(std::ostream& os, const ltsy::CognitiveAttitude& ca) {
    os << ca.symbol;
    return os;
}
#endif

