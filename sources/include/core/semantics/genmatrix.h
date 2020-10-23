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
    class GenMatrixVarAssignment : public std::enable_shared_from_this<GenMatrixVarAssignment> {
        private:

            std::shared_ptr<GenMatrix> _nmatrix_ptr;
            std::map<Prop, int> _valuation_map;
            
        public:

            GenMatrixVarAssignment() : _nmatrix_ptr {nullptr} {}
            /**
             * Initialize a valuation, by indicating
             * the corresponding generalized matrix
             * and the map that assigns to each propositional
             * variable a truth-value.
             * */
            GenMatrixVarAssignment(decltype(_nmatrix_ptr) nmatrix_ptr,
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

            bool operator==(const GenMatrixVarAssignment& other) const {
                return _valuation_map == other._valuation_map;
            }

            std::stringstream print() const {
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

    /* Based on a variable assignment, visit a formula to
     * determine the set of all truth values it
     * may assume, considering every possible
     * valuation.
     *
     * @author Vitor Greati
     * */
    class GenMatrixPossibleValuesCollector : public FormulaVisitor<std::set<int>> {
        private:
            std::shared_ptr<GenMatrixVarAssignment> _matrix_valuation_ptr;

        public:

            GenMatrixPossibleValuesCollector(decltype(_matrix_valuation_ptr) matrix_valuation_ptr) :
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
    class GenMatrixVarAssignmentGenerator {
        
        private:
            std::shared_ptr<GenMatrix> _matrix; 
            std::vector<std::shared_ptr<Prop>> _props;
            int _current_index = 0;
            int _total_valuations;

        public:

            GenMatrixVarAssignmentGenerator(decltype(_matrix) nmatrix, decltype(_props) props) : _props {props} {
                std::atomic_store(&_matrix, nmatrix);
                auto number_props = _props.size();
                auto nvalues = _matrix->values().size();
                _total_valuations = std::pow(nvalues, number_props); 
            }

            bool has_next() {
                return _current_index < _total_valuations;
            }

            std::shared_ptr<GenMatrixVarAssignment> next() {
                if (not has_next())
                    throw std::logic_error("no valuations to generate");
                auto images = utils::tuple_from_position(_matrix->values().size(), _props.size(), _current_index);
                std::vector<std::pair<Prop, int>> val_map;
                for (int i = 0; i < _props.size(); ++i) {
                    Prop p = *_props[i];
                    val_map.push_back({p, images[i]});
                }
                ++_current_index;
                return std::make_shared<GenMatrixVarAssignment>(_matrix, val_map); 
            }
    };

    /* A generalized matrix valuation. 
     * Since in such matrices the interpretations may
     * be non-deterministic or even partial,
     * a valuation will also point to a 
     * determinization of an interpretation
     * (only singleton or empty set in each
     * position). Then, an evaluator will
     * use such an interpretation
     * for evaluating a formula.
     *
     * @author Vitor Greati
     * */
    class GenMatrixValuation {
    
        private:

            std::shared_ptr<GenMatrixVarAssignment> _var_assignment; /* an assignment to variables */
            std::shared_ptr<GenMatrix> _nmatrix_ptr; /* pointer to a PNmatrix */
            std::shared_ptr<SignatureTruthInterp<std::set<int>>> 
                _interpretation; /* pointer to a determinized truth interpretation*/

        public:

            GenMatrixValuation(decltype(_var_assignment) var_assignment,
                    decltype(_nmatrix_ptr) nmatrix_ptr, 
                    decltype(_interpretation) interpretation) :
            _var_assignment {var_assignment}, _nmatrix_ptr {nmatrix_ptr}, _interpretation {interpretation} 
            {/* empty */}

            inline const decltype(_nmatrix_ptr) nmatrix_ptr() const {
                return _nmatrix_ptr;
            }
    
    };


    /* Determinizer for a given (partial) non-deterministic interpretation.
     *
     * Given a (partial) non-deterministic truth-table, generate
     * all possible (partial) deterministic interpretations.
     *
     * @author Vitor Greati
     * */
    class PartialDeterministicTruthTableGenerator {
    
        private:
            std::shared_ptr<TruthTable<std::set<int>>> _tt_start;
            std::shared_ptr<TruthTable<std::set<int>>> _current;
            std::vector<std::vector<std::set<int>>> _possible_images;
            std::vector<int> _current_indices;
            int qtd_in_max = 0;
            bool finished = false;
            bool first = true;

        public:

            PartialDeterministicTruthTableGenerator(decltype(_tt_start) tt_start)
                : _tt_start {tt_start} {
                 _current = std::make_shared<TruthTable<std::set<int>>>(tt_start->nvalues(),
                         tt_start->arity());
                // fill in the possible images
                auto determinants = _tt_start->get_determinants();
                for (const auto& d : determinants) {
                    std::vector<std::set<int>> imgs;
                    if (d.get_last().empty())
                        imgs.push_back({});
                    else
                        for (const auto& e : d.get_last())
                            imgs.push_back({e});
                    _possible_images.push_back(imgs);
                    _current_indices.push_back(0);
                }
                reset();
            }

            /* Inform if there is a next truth table
             * to be generated.
             * */
            bool has_next() {
                return not finished;
            }

            /* Ask for next truth table.
             *
             * Uses a binary counter algorithm to generate
             * the next element. Throws an exception
             * if there is no next table.
             *
             * @return the next truth table
             * */
            decltype(_current) next() {
                if (not has_next())
                    throw std::logic_error("next called in finished generator");
                if (first) {
                    first = false;
                } else {
                    int k = _possible_images.size() - 1;
                    auto has_next_in_position = [&](int k) { 
                        return _current_indices[k] < (_possible_images[k].size() - 1);
                    };
                    while (k >= 0 and not has_next_in_position(k)) {
                        if (_possible_images[k].size() > 1) {
                            _current_indices[k] = 0;
                            _current->set(k, _possible_images[k][0]);
                            --qtd_in_max;
                        }
                        --k;
                    }
                    if (k != -1) {
                        ++_current_indices[k]; 
                        if (not has_next_in_position(k)) 
                            ++qtd_in_max;
                        _current->set(k, _possible_images[k][_current_indices[k]]);
                    }
                }
                if (qtd_in_max == _current_indices.size())
                    finished = true;
                return _current;
            }

            /* Reset the generator.
             * */
            void reset() {
                finished = false;
                first = true;
                qtd_in_max = 0;
                // create first element
                for (int i = 0; i < _possible_images.size(); ++i) {
                    _current->set(i, _possible_images[i][0]);
                    _current_indices[i] = 0;
                    if (_possible_images[i].size() == 1) qtd_in_max++;
                }
            }
    };

    /* Determinize an entire (partial) non-deterministic 
     * truth interpretation for a given signature.
     *
     * Given a (partial) non-deterministic truth-table, generate
     * all possible (partial) deterministic interpretations.
     * */
    class PartialDeterministicTruthInterpGenerator {
        private:
            std::shared_ptr<SignatureTruthInterp<std::set<int>>> _start_truth_interp;
            std::map<Symbol, PartialDeterministicTruthTableGenerator> _generators;
            std::shared_ptr<SignatureTruthInterp<std::set<int>>> _current;

            void initialize_generators() {
                auto signature = _start_truth_interp->signature();
                for (auto [symbol, connective] : (*signature)) {
                    _generators.insert({symbol, 
                            PartialDeterministicTruthTableGenerator(
                                    this->_start_truth_interp->get_interpretation(symbol)->truth_table())}); 
                } 
            }

            std::shared_ptr<SignatureTruthInterp<std::set<int>>> truth_interp_from_generators() {
                auto signature = _start_truth_interp->signature();
                auto sti = std::make_shared<SignatureTruthInterp<std::set<int>>>(signature);
                for (auto& [symbol, gen] : _generators)
                    sti->try_interpret(
                            std::make_shared<TruthInterp<std::set<int>>>((*signature)[symbol], gen.next()), 
                            true);
                (*(_generators.begin())).second.reset();
                return sti;
            }

        public:

            PartialDeterministicTruthInterpGenerator(decltype(_start_truth_interp) start_truth_interp) 
                : _start_truth_interp {start_truth_interp} {
                initialize_generators();    
                reset();
            }

            /* Produce next interpretation.
             * */
            decltype(_current) next() {
                if (not has_next())
                    throw std::logic_error("no truth interpretation available");
                
                auto signature = _start_truth_interp->signature();

                auto make_truth_interp = [&](const Symbol& symbol, 
                        std::shared_ptr<TruthTable<std::set<int>>> tt) {
                    return std::make_shared<TruthInterp<std::set<int>>>((*signature)[symbol], tt);
                };

                for (auto& [symbol, generator] : _generators) {
                    if (generator.has_next()) {
                        _current->try_interpret(make_truth_interp(symbol, generator.next()), true);
                        break;
                    } 
                    generator.reset();
                    _current->try_interpret(make_truth_interp(symbol, generator.next()), true);
                }
                return _current;
            }

            /* Reset the generator.
             * */
            void reset() {
                for (auto& [s, g] : _generators)
                    g.reset();
                _current = truth_interp_from_generators();
            }
            
            /* Check if there is a next interpretation
             * to produce.
             * */
            bool has_next() {
                for (auto& [s, g] : _generators) {
                    if (g.has_next()) 
                        return true;
                }
                return false;
            }
    };

    /* Check if a sequent is valid on a given
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
                GenMatrixVarAssignment val; 
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

            /* Given a variable assignment, determines
             * whether all possible values given
             * such assignment are contained in a given
             * distinguished set.
             *
             * @author Vitor Greati
             * */
            std::pair<bool, std::optional<std::set<std::shared_ptr<Formula>>>> 
            is_fmla_set_valid_under_assignment(const GenMatrixVarAssignment val, 
                            const std::set<std::shared_ptr<Formula>>& fmls, const std::set<int>& dset) const {
                std::set<std::shared_ptr<Formula>> fail_fmls;
                for (const auto& f : fmls) {
                   GenMatrixPossibleValuesCollector collector {std::make_shared<GenMatrixVarAssignment>(val)};
                   auto fmla_values = f->accept(collector);     
                   if (!utils::is_subset(fmla_values, dset))
                       fail_fmls.insert(f);
                }
                if (fail_fmls.empty()) return {true, std::nullopt};
                else return std::pair{false, fail_fmls};               
            }

            bool
            is_valid_under_assignment(const GenMatrixVarAssignment& val, const NdSequent<FmlaContainerT>& seq) const {
                 for (int i {0}; i < seq.dimension(); ++i) {
                     auto is_model_result = is_fmla_set_valid_under_assignment(val, seq[i], _d_sets[i]);
                     if (not is_model_result.first) return true;
                 }
                 return false;
            }

            std::pair<bool, std::optional<std::vector<CounterExample>>> 
            is_valid(const NdSequent<FmlaContainerT>& seq, int max_counter_examples=1) const { 
                std::vector<CounterExample> counter_examples;
                auto props_set = seq.collect_props();
                std::vector<Prop> props {props_set.begin(), props_set.end()};
                ltsy::GenMatrixVarAssignmentGenerator generator {_matrix, props};
                while (generator.has_next()) {
                    auto val = generator.next();
                    if (not is_valid_under_assignment(*val, seq)) {
                        counter_examples.push_back({*val});
                        if (counter_examples.size() >= max_counter_examples)
                            break;
                    }
                }
                if (counter_examples.empty())
                    return {true, std::nullopt};
                else
                    return {false, counter_examples};
            }

            std::pair<bool, std::optional<std::vector<CounterExample>>> 
            is_rule_satisfiability_preserving(const NdSequentRule<FmlaContainerT>& rule, 
                    int max_counter_examples=1) const { }

            /* Test if a rule preserves validity under
             * every possible valuations.
             * */
            std::pair<bool, std::optional<std::vector<CounterExample>>> 
            is_rule_validity_preserving(const NdSequentRule<FmlaContainerT>& rule, 
                    int max_counter_examples=1) const { 
                std::vector<CounterExample> counter_examples;
                auto props_set = rule.collect_props();
                std::vector<std::shared_ptr<Prop>> props {props_set.begin(), props_set.end()};
                ltsy::GenMatrixVarAssignmentGenerator generator {_matrix, props};
                while (generator.has_next()) {
                    // check validity of premisses
                    bool premisses_valid = true;
                    auto val = generator.next();
                    for (const auto& p : rule.premisses()) {
                        if (not is_valid_under_assignment(*val, p)) {
                            premisses_valid = false;
                            break;
                        }
                    }
                    // check non-validity of conclusions
                    bool conclusions_not_valid = true;
                    if (premisses_valid) {
                        for (const auto& c : rule.conclusions()) {
                            if (is_valid_under_assignment(*val, c)) {
                                conclusions_not_valid = false;
                                break;
                            }
                        }
                    }
                    if (premisses_valid and conclusions_not_valid)
                        counter_examples.push_back(CounterExample{*val});
                    if (counter_examples.size() >= max_counter_examples)
                        break;
                }
                if (counter_examples.empty())
                    return std::make_pair<bool, std::optional<std::vector<CounterExample>>>(true, std::nullopt);
                else
                    return std::make_pair<bool, std::optional<std::vector<CounterExample>>>(false, 
                        std::make_optional<std::vector<CounterExample>>(counter_examples));
            }
    
    };

};

#endif
