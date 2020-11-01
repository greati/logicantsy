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

    bool Formula::operator<(const Formula& p1) const {
        StrictWeakOrderingFormulaVisitor orderTester (this);
        return p1.accept(orderTester);
    }

    bool Formula::operator==(const Formula& p1) const {
        EqualityFormulaVisitor equalityTester (this);
        return p1.accept(equalityTester);
    }

    bool is_subset(const FmlaSet& f1, const FmlaSet& f2) {
        if (f1.size() > f2.size()) return false;
        for (auto& e : f1) 
            if (f2.find(e) == f2.end()) 
                return false;
        return true;
    }
}
