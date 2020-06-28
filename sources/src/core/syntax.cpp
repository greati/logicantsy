#include <unordered_set>
#include "core/syntax.h"
#include <ostream>

namespace ltsy {
    std::ostream& operator<<(std::ostream& os, ltsy::Formula& f) {
        ltsy::FormulaPrinter p;
        f.accept(p);
        os << p.get_string();
        return os;
    }
}
