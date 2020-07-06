#ifndef __ND_SEQUENTS_DETERM__
#define __ND_SEQUENTS_DETERM__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/attitudes.h"
#include "core/proof-theory/ndsequents.h"

namespace ltsy {

    /**
     * Determinizer based on multi-dimensional pure and simple 
     * sequents w.r.t. a single connective #,
     * i.e., the formulas ocurring in the sequents come from
     * a propositional base of size arity(2) or are
     * the compound of # in its most general format.
     * We assume here an N-dimensional sequent,
     * each position being associated to
     * a cognitive attitude, each attitude having
     * an associated set of truth-values.
     *
     * For example, for a binary connective # and propositional
     * base {p,q},
     * allowed formulas are p, q, and p#q.
     * */
    class NdSequentTruthTableDeterminizer {

        private:
            int _nvalues;
            std::set<int> _all_values;
            Connective _connective;
            std::vector<Prop> _props;
            std::shared_ptr<Compound> _compound;
            TruthTable<std::set<int>> _table;
            std::vector<CognitiveAttitude> _attitudes;

            void _remove_determinants_by_pos_attitude(const CognitiveAttitude& attitude, int pos, 
                    std::set<Determinant<std::set<int>>>& determinants) {
                for (auto it = determinants.begin(); it != determinants.end();) {
                    auto det_args = (*it).get_args(); 
                    if (attitude.values.find(det_args[pos]) == attitude.values.end())
                           it = determinants.erase(it); 
                    else it++;
                } 
            }

            bool _determine_by_dimension(int dimension, const NdSequent<std::set>& sequent, 
                    std::set<Determinant<std::set<int>>>& determinants) {
                for (int i = 0; i < _props.size(); ++i) {
                    Formula& p = _props[i];
                    if (sequent.is_in(dimension, p))
                        _remove_determinants_by_pos_attitude(_attitudes[dimension], i, determinants);
                }
                if (sequent.is_in(dimension, *_compound)) {
                    for (auto it = determinants.begin(); it != determinants.end();) {
                        auto det = (*it);
                        std::set<int> cm_set;
                        std::set_difference(_all_values.begin(), _all_values.end(),
                                            _attitudes[dimension].values.begin(), 
                                            _attitudes[dimension].values.end(), 
                                            std::inserter(cm_set, cm_set.end()));
                        std::set<int> new_cm_set;
                        // ugly conversion unord set to set
                        std::set<int> det_val_set;
                        for (auto e : det.get_last())
                            det_val_set.insert(e);
                        std::set_difference(
                                det_val_set.begin(), det_val_set.end(),
                                cm_set.begin(), cm_set.end(),
                                std::inserter(new_cm_set, new_cm_set.end()));
                        std::set<int> s (new_cm_set.begin(), new_cm_set.end());
                        det.set_last(s);
                        it = determinants.erase(it);
                        determinants.insert(it, det);
                    }
                    return true;
                }
                return false;
            }

            void _determine_by_sequent(const NdSequent<std::set>& sequent) {
                auto dimension = sequent.dimension();     
                auto counter_model_determinants = 
                    generate_fully_nd_table(_nvalues, _connective.arity()).get_determinants();
                bool compound_has_appeared = false;
                // produce counter-model determinants
                for (auto i = 0; i < dimension; ++i)
                    compound_has_appeared |= _determine_by_dimension(i, sequent, counter_model_determinants); 
                _table.update(counter_model_determinants, 
                        [](std::set<int> current, std::set<int> counter_models) -> std::set<int> {
                            return utils::set_difference(current, counter_models);
                        }
                );
            }

        public:

            NdSequentTruthTableDeterminizer(
                    int nvalues, 
                    decltype(_props) props,
                    decltype(_connective) connective,
                    decltype(_attitudes) attitudes) 
                : _nvalues {nvalues}, _props {props}, _connective {connective}, _attitudes {attitudes} {
                std::vector<std::shared_ptr<Formula>> props_args;
                for (auto p : props)
                    props_args.push_back(std::dynamic_pointer_cast<Formula>(std::make_shared<Prop>(p)));
                _compound = std::make_shared<Compound>(std::make_shared<Connective>(_connective), props_args);
                _table = generate_fully_nd_table(_nvalues, _connective.arity());
                for (auto i = 0; i < _nvalues; ++i) _all_values.insert(i);
            }

            void determine(const std::vector<NdSequent<std::set>>& sequents) {
                for (const auto& s : sequents)
                    _determine_by_sequent(s);
            } 

            decltype(_table) table() const { return _table; }

    };

};

#endif
