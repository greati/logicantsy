#include "core/semantics/attitudes.h"

using namespace ltsy;


std::ostream& operator<<(std::ostream& os, const ltsy::CognitiveAttitude& ca) {
    os << ca.symbol;
    return os;
}



