#ifndef __ATTITUDES__
#define __ATTITUDES__

#include <set>
#include "core/common.h"
#include <iostream>
#include <memory>
#include <map>

namespace ltsy {

    struct CognitiveAttitude {
        Symbol symbol;
        std::set<int> values;

        CognitiveAttitude() {/*empty*/}
        CognitiveAttitude(const Symbol& symbol, const decltype(values)& values) 
            : values {values}, symbol {symbol} {/*empty*/}

        inline bool operator<(const CognitiveAttitude& other) const {
            return this->symbol < other.symbol;
        }
        inline bool operator==(const CognitiveAttitude& other) const {
            return this->symbol == other.symbol;
        }
    };

}

std::ostream& operator<<(std::ostream& os, const ltsy::CognitiveAttitude& ca);

#endif

