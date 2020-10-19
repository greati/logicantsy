#ifndef __GEN_MATRIX__
#define __GEN_MATRIX__

#include "core/syntax.h"
#include "core/common.h"
#include "core/combinatorics/combinations.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/nmatrices.h"
#include "core/proof-theory/ndsequents.h"
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
    class GenMatrixValuation : public std::enable_shared_from_this<GenMatrixValuation> {
        private:

            std::shared_ptr<GenMatrix> _nmatrix_ptr;
            std::map<Prop, int> _valuation_map;
            
        public:

            GenMatrixValuation() : _nmatrix_ptr {nullptr} {/**/}
            
            /**
             * Initialize a valuation, by indicating
             * the corresponding generalized matrix
             * and the map that assigns to each propositional
             * variable a truth-value.
             * */
            GenMatrixValuation(decltype(_nmatrix_ptr) nmatrix_ptr,
                    const std::vector<std::pair<Prop, int>>& mappings)
                : _nmatrix_ptr {nmatrix_ptr}
                {
                //std::atomic_store(&(this->_nmatrix_ptr), std::move(nmatrix_ptr));

                auto values = _nmatrix_ptr->values();
               
                for (auto& mapping : mappings) {
                    auto [p, v] = mapping;
                    if (v >= 0 and v < values.size()) {
                        _valuation_map.insert(mapping);
                    } else {
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

            /* Checks if the valuation is a model for the given set
             * of formulas. The criteria is that, for each formula,
             * one of the possible values is inside a given
             * distinguished set.
             *
             * @param fmls set of formulas
             * @param dset distinguished set
             * */
            std::pair<bool, std::optional<std::set<std::shared_ptr<Formula>>>> 
            is_model(const std::set<std::shared_ptr<Formula>>& fmls,
                    const std::set<int>& dset);
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


    /**
     * Generator of generalized matrix valuations.
     * */
    class GenMatrixValuationGenerator {
        
        private:
            std::shared_ptr<GenMatrix> _matrix; 
            std::vector<std::shared_ptr<Prop>> _props;
            int _current_index = 0;
            int _total_valuations;

        public:

            GenMatrixValuationGenerator(decltype(_matrix) nmatrix, decltype(_props) props) : _props {props} {
                std::atomic_store(&_matrix, nmatrix);
                auto number_props = _props.size();
                auto nvalues = _matrix->values().size();
                _total_valuations = std::pow(nvalues, number_props); 
            }

            bool has_next() {
                return _current_index < _total_valuations;
            }

            GenMatrixValuation next() {
                if (not has_next())
                    throw std::logic_error("no valuations to generate");
                auto images = utils::tuple_from_position(_matrix->values().size(), _props.size(), _current_index);
                std::vector<std::pair<Prop, int>> val_map;
                for (int i = 0; i < _props.size(); ++i) {
                    Prop p = *_props[i];
                    val_map.push_back({p, images[i]});
                }
                ++_current_index;
                return GenMatrixValuation(_matrix, val_map); 
            }
    };

    /* Chack if a sequent is valid on a given
     * generalized matrix. Validity is defined in the form
     * of: there is no valuation v such that
     * v(G1) \subseteq S1 and
     * v(G2) \subseteq S2 ...
     * and v(GN) \subseteq SN.
     *
     * We must indicate what are the sets
     * of the matrix that will correspond to each position
     * of the sequent.
     * If the sequent has dimension N, then 
     * the user must specify a vector P of size N
     * such that the distinguished set D_i will be in position P[i]
     * of the sequent, then V\D_i will be in position P[i+1],
     * for i even.
     * */
    template<template<class...> typename FmlaContainerT>
    class NdSequentGenMatrixValidator {

        private:
            std::shared_ptr<GenMatrix> _matrix; 
            std::vector<int> _sequent_set_correspondence;
            std::vector<std::set<int>> _d_sets;
    
        public:
            /**
             * Represents a counter-example for
             * a given sequent.
             * */
            struct CounterExample {
                GenMatrixValuation val; 
                CounterExample(decltype(val) _val) : val {_val} {}
            };

            /* Constructor.
             *
             * @param matrix pointer to generalized matrix
             * @param sequent_set_correspondence map indicating the link between
             * sequent positions and matrix sets
             * @param infer_complements if true, modify sequent_set_correspondence 
             * such the odd positions points to the complement of
             * the sets.
             * */
            NdSequentGenMatrixValidator(decltype(_matrix) matrix,
                    const decltype(_sequent_set_correspondence)& sequent_set_correspondence,
                    bool infer_complements=true) :
               _matrix {matrix}, _sequent_set_correspondence {sequent_set_correspondence} {
                for (int i = 0; i < sequent_set_correspondence.size(); ++i) {
                    _d_sets.push_back(matrix->distinguished_sets()[i]);
                    if (infer_complements)
                        _d_sets.push_back(utils::set_difference(matrix->values(), matrix->distinguished_sets()[i]));
                }
            }

            bool
            is_satisfied(GenMatrixValuation& val, const NdSequent<FmlaContainerT>& seq) const {
                 for (int i {0}; i < seq.dimension(); ++i) {
                     auto [is_model, counter_fmlas_opt] = val.is_model(seq[i], _matrix->distinguished_sets()[i]);
                     if (not is_model) return true;
                 }
                 return false;
            }

            std::pair<bool, std::optional<std::vector<CounterExample>>> 
            is_valid(const NdSequent<FmlaContainerT>& seq, int max_counter_examples=1) const { 
                std::vector<CounterExample> counter_examples;
                auto props_set = seq.collect_props();
                std::vector<Prop> props {props_set.begin(), props_set.end()};
                ltsy::GenMatrixValuationGenerator generator {_matrix, props};
                while (generator.has_next()) {
                    auto val = generator.next();
                    if (not is_satisfied(val, seq)) {
                        counter_examples.push_back({val});
                        if (counter_examples.size() >= max_counter_examples)
                            break;
                    }
                }
                if (counter_examples.empty())
                    return {true, std::nullopt};
                else
                    return {false, counter_examples};
            }

            //std::pair<bool, GenMatrixValuation> 
            //std::pair<bool, std::vector<GenMatrixValuation>> 
            GenMatrixValuation is_rule_sound(int max_counter_examples=1) const { 
                //std::vector<CounterExample> counter_examples;
                //auto props_set = rule.collect_props();
                //std::vector<std::shared_ptr<Prop>> props {props_set.begin(), props_set.end()};
               // ltsy::GenMatrixValuationGenerator generator {_matrix, props};
               //while (generator.has_next()) {
               //    // check satisfiability of premisses
               //    bool premisses_satisfied = true;
               //    auto val = generator.next();
               //    for (const auto& p : rule.premisses()) {
               //        if (not is_satisfied(val, p)) {
               //            premisses_satisfied = false;
               //            break;
               //        }
               //    }
               //    // check non-validity of conclusions
               //    bool conclusions_not_satisfied = true;
               //    if (premisses_satisfied) {
               //        for (const auto& c : rule.conclusions()) {
               //            if (is_satisfied(val, c)) {
               //                conclusions_not_satisfied = false;
               //                break;
               //            }
               //        }
               //    }
               //    if (premisses_satisfied and conclusions_not_satisfied)
               //        counter_examples.push_back(CounterExample{val});
               //}
               //if (counter_examples.empty())
               //    return {true, std::nullopt};
               //else
               //    return {false, std::make_optional<std::vector<CounterExample>>(counter_examples)};
            }
    
    };

};

#endif
