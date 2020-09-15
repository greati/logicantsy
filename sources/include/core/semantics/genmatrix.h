#ifndef __GEN_MATRIX__
#define __GEN_MATRIX__

#include "core/syntax.h"
#include "core/common.h"
#include "core/combinatorics/combinations.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/nmatrices.h"
#include <set>

namespace ltsy {

    class GenMatrix {
    
        private:
            std::set<int> _values;
            std::vector<std::set<int>> _distinguished_sets;
            std::shared_ptr<Signature> _signature;
            std::shared_ptr<SignatureTruthInterp<std::set<int>>> _interpretation;

        public:

            GenMatrix(const decltype(_values)& values, const decltype(_distinguished_sets)& distinguished_sets, 
                    decltype(_signature) signature,
                    decltype(_interpretation) interpretation)
                : _values {values}, _distinguished_sets {distinguished_sets}, 
                _signature {signature}, _interpretation {interpretation} {
            }

            GenMatrix(const decltype(_values)& values, const decltype(_distinguished_sets)& distinguished_sets)
                    : GenMatrix {values, distinguished_sets, nullptr, nullptr} {}

            inline decltype(_values) values() const { return _values; }

            inline decltype(_distinguished_sets) distinguished_sets() const { return _distinguished_sets; }

            inline decltype(_interpretation) interpretation() const {
                return _interpretation;
            }
             
            inline decltype(_signature) signature() const { return _signature; }
    };

};

#endif
