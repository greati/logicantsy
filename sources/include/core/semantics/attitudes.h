#ifndef __ATTITUDES__
#define __ATTITUDES__

#include <set>
#include "core/common.h"

namespace ltsy {
    struct CognitiveAttitude {
        Symbol symbol;
        std::set<int> values;

        CognitiveAttitude(const Symbol& symbol, const decltype(values)& values) : values {values}, symbol {symbol} {/*empty*/}
    };
}

#endif

