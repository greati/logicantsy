#ifndef __GEN_MATRIX__
#define __GEN_MATRIX__

#include "core/syntax.h"
#include "core/common.h"
#include "core/combinatorics/combinations.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/nmatrices.h"
#include <set>

namespace ltsy {
    
    /**
     * Represents a generalized version of
     * matrices, in the following sense:
     * - we may define more than one distinguished set (thus allowing
     *   the creation of q-matrices, p-matrices, B-matrices...)
     * - we may interpret the connectives using (partial) non-deterministic
     *   truth-tables.
     *
     * @author Vitor Greati
     * */
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

    /**
     * A valuation over an generalized matrix, specified
     * by assignments to each propositional variable.
     * Although it is formally expected of such
     * a valuation to provide a single truth-value
     * to each formula, we implement this visitor
     * to answer with all possible values, since we
     * are dealing here with non-truth-funcional semantics
     * (see the theory of NMatrices).
     *
     * @author Vitor Greati
     * */
    class GenMatrixValuation {
        private:

            std::shared_ptr<GenMatrix> _nmatrix_ptr;
            std::map<Prop, int> _valuation_map;
            
        public:
            
            /**
             * Initialize a valuation, by indicating
             * the corresponding generalized matrix
             * and the map that assigns to each propositional
             * variable a truth-value.
             * */
            GenMatrixValuation(decltype(_nmatrix_ptr) nmatrix_ptr,
                    const std::vector<std::pair<Prop, int>>& mappings)
                {

                std::atomic_store(&(this->_nmatrix_ptr), std::move(nmatrix_ptr));

                auto values = _nmatrix_ptr->values();
               
                for (auto& mapping : mappings) {
                    auto [p, v] = mapping;
                    if (v >= 0 and v < values.size()) {
                        _valuation_map.insert(mapping);
                    } else {
                        std::cout << v;
                        throw std::invalid_argument(INVALID_TRUTH_VALUE_EXCEPTION);
                    }
                } 
            }

            std::stringstream print() {
                std::stringstream ss;
                for (auto& [k, v] : _valuation_map) {
                    ss << k.symbol() << " -> " << v << std::endl;
                }
                return ss;
            }

            inline const decltype(_nmatrix_ptr) nmatrix_ptr() const {
                return _nmatrix_ptr;
            }
            
            inline int operator()(const Prop& p) {
                return _valuation_map[p];
            }
    };

    /* Based on a valuation, visit a formula to
     * determine its truth value (it is the 
     * unique homomorphic extension of the 
     * given valuation).
     *
     * @author Vitor Greati
     * */
    class GenMatrixEvaluator : public FormulaVisitor<std::set<int>> {
        private:
            std::shared_ptr<GenMatrixValuation> _matrix_valuation_ptr;

        public:

            GenMatrixEvaluator(decltype(_matrix_valuation_ptr) matrix_valuation_ptr) :
                _matrix_valuation_ptr {matrix_valuation_ptr} {/* empty */}

            std::set<int> visit_prop(Prop* prop) override {
                if (prop != nullptr) {
                    return std::set<int>{(*_matrix_valuation_ptr)(*prop)};
                } else throw std::logic_error("proposition points to null");
            }

            std::set<int> visit_compound(Compound* compound) override {
               if (compound != nullptr) {
                   auto connective = compound->connective();
                   auto conn_interp = 
                       _matrix_valuation_ptr->nmatrix_ptr()->interpretation()
                           ->get_interpretation(connective->symbol());
                   auto components = compound->components();
                   std::vector<std::set<int>> args;
                   for (auto component : components) {
                        args.push_back(component->accept(*this));
                   }
                   auto possible_arguments = utils::cartesian_product(args);
                   std::set<int> result;
                   for (const auto& arg : possible_arguments) {
                      auto conn_values = conn_interp->at(arg);
                      result.insert(conn_values.begin(), conn_values.end());
                   }
                   return result;
               } else throw std::logic_error("compound points to null");
            }
    };

};

#endif
