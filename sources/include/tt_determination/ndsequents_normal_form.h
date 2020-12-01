#ifndef __NDSEQ_NORMAL_FORM__
#define __NDSEQ_NORMAL_FORM__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/attitudes.h"
#include "core/semantics/judgment_values_corr.h"
#include "core/proof-theory/ndsequents.h"
#include "core/utils.h"

namespace ltsy {

    /**
     * Produce a set of sequents that determinize
     * a truth table, according to notions
     * of cognitive attitudes.
     *
     * @author Vitor Greati
     * */
    class NdSequentsFromTruthTable {

        private:

            size_t _sequent_dim = 4;
            std::set<int> _all_values;
            Connective _connective;
            std::vector<std::shared_ptr<Prop>> _props;
            std::shared_ptr<Compound> _compound;
            std::vector<CognitiveAttitude> _judgements;
            std::map<CognitiveAttitude, int> _judg_position;
            std::shared_ptr<JudgementValueCorrespondence> _judg_value_correspondence;

        public:

            NdSequentsFromTruthTable(const decltype(_all_values)& all_values,
                    const decltype(_connective)& connective,
                    const decltype(_props)& props,
                    const decltype(_compound)& compound,
                    const decltype(_judgements)& judgements,
                    const decltype(_judg_value_correspondence)& judg_value_correspondence)
                : _all_values {all_values}, _props {props}, _compound {compound},
                _judgements {judgements}, _judg_value_correspondence {judg_value_correspondence} 
            {
                for (size_t i {0}; i < _judgements.size(); ++i)
                    _judg_position[_judgements[i]] = i;
            }


            std::vector<NdSequent<std::set>> axiomatize(const NDTruthTable& table) {
                std::vector<NdSequent<std::set>> sequents;
                for (const auto& d : table.get_determinants()) {
                   sequents_from_determinants(d, sequents); 
                } 
                return sequents;
            } 

        private:

            void sequents_from_determinants(const Determinant<std::set<int>>& det,
                    std::vector<NdSequent<std::set>>& sequents) {
                auto response_set = det.get_last(); 
                auto avoid_set = utils::set_difference(this->_all_values, response_set);
                auto avoid_set_size = avoid_set.size();
                auto all_values_size = _all_values.size();
                // fully non-deterministic cell, no sequents is necessary
                if (avoid_set_size == 0)
                    return;
                // partiality
               //if (avoid_set_size == all_values_size) {
               //    NdSequent<std::set> seq {_sequent_dim, {}};
               //    place_variables(seq, det);
               //    sequents.push_back(seq);
               //}
                // other cases
                else {
                    auto judgs_from_set = _judg_value_correspondence->judgements_from_set_values(avoid_set); 
                    for (const auto& judgs : judgs_from_set) {
                        NdSequent<std::set> seq {_sequent_dim, {}};
                        place_in_sequent(seq, _compound, judgs); 
                        place_variables(seq, det);
                        sequents.push_back(seq);
                    }
                }
            };

            void place_variables(NdSequent<std::set>& seq, const Determinant<std::set<int>>& det) {
                const auto& det_args = det.get_args();
                for (size_t i {0}; i < det_args.size(); ++i) {
                    auto dv = det_args[i];
                    auto judgs = _judg_value_correspondence->judgements_from_value(dv);
                    this->place_in_sequent(seq, _props[i], judgs);
                }
            }

            /**
             * Place a given formula in positions corresponding to the
             * given judgements.
             * */
            void place_in_sequent(NdSequent<std::set>& sequent,
                    std::shared_ptr<Formula> fmla, const std::set<CognitiveAttitude>& judgs) {
                for (const auto& j : judgs) {
                    auto pos = _judg_position[j];
                    sequent[pos].insert(fmla);
                } 
            }
    };
};

#endif
