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
            Connective _connective;
            std::vector<Prop> _props;
            std::shared_ptr<Compound> _compound;
            TruthTable<std::unordered_set<int>> _table;
            std::vector<CognitiveAttitude> _attitudes;

            void _remove_determinants_by_pos_attitude(const CognitiveAttitude& attitude, int pos, 
                    std::set<Determinant<std::unordered_set<int>>>& determinants) {
                for (auto it = determinants.begin(); it != determinants.end();) {
                    auto det_args = (*it).get_args(); 
                    if (attitude.values.find(det_args[pos]) == attitude.values.end())
                           it = determinants.erase(it); 
                    else it++;
                } 
            }

            bool _determine_by_dimension(int dimension, const NdSequent<std::set>& sequent, 
                    std::set<Determinant<std::unordered_set<int>>>& determinants) {
                for (int i = 0; i < _props.size(); ++i) {
                    Formula& p = _props[i];
                    if (sequent.is_in(dimension, p))
                        _remove_determinants_by_pos_attitude(_attitudes[dimension], i, determinants);
                }
                std::cout << "After propositional" << std::endl;
                std::cout << determinants << std::endl;
                if (sequent.is_in(dimension, *_compound)) {
                    for (auto it = determinants.begin(); it != determinants.end();) {
                        auto det = (*it);
                        det.get_last().insert(_attitudes[dimension].values.begin(), _attitudes[dimension].values.end());
                        it = determinants.erase(it);
                        determinants.insert(it, det);
                    }
                    return true;
                }
                return false;
            }

            void _determine_by_sequent(const NdSequent<std::set>& sequent) {
                auto dimension = sequent.dimension();     
                auto counter_model_determinants = generate_fully_partial_table(_nvalues, _connective.arity()).get_determinants();
                bool compound_has_appeared = false;
                // produce counter-model determinants
                for (auto i = 0; i < dimension; ++i)
                    compound_has_appeared |= _determine_by_dimension(i, sequent, counter_model_determinants); 
                // if the compound has not appeared, set the counter-model determinants to be fully ND
                if (not compound_has_appeared)
                    for (auto it = counter_model_determinants.begin(); it != counter_model_determinants.end();) {
                        auto det = (*it);
                        std::unordered_set<int> s;
                        for (int i = 0; i < _nvalues; ++i)
                            s.insert(i);
                        det.set_last(s);
                        it = counter_model_determinants.erase(it);
                        counter_model_determinants.insert(it, det);
                    }
                // update table based on determination
                _table.update(counter_model_determinants, 
                        [](std::unordered_set<int> current, std::unordered_set<int> counter_models) -> std::unordered_set<int> {
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
            }

            void determine(const std::vector<NdSequent<std::set>>& sequents) {
                for (const auto& s : sequents)
                    _determine_by_sequent(s);
            } 

            decltype(_table) table() const { return _table; }

    };

};

#endif
