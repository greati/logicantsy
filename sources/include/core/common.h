#ifndef __COMMON__
#define __COMMON__

#include <unordered_set>
#include <set>
#include <vector>
#include <string>
#include <ostream>
#include <variant>
#include <map>
#include "external/prettyprinter/containers.h"

namespace ltsy {
    using Symbol = std::string;
    using Arity = int;

    using PrimKeyType = std::variant<char, int, std::string>;

    /* Abstract class for printable objects.
     *
     * Maintains a map that translates primitive values into
     * string ones. The purpose is to allow the user to
     * provide such translations.
     * */
    class PrintableI {
        public:
            virtual std::string print() const = 0;
    };

};

#endif
