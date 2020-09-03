#ifndef __NDSEQ_NORMAL_FORM__
#define __NDSEQ_NORMAL_FORM__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/attitudes.h"
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

            std::set<int> _all_values;
            Connective _connective;
            std::vector<Prop> _props;
            std::shared_ptr<Compound> _compound;
            std::vector<CognitiveAttitude> _attitudes;
            NDTruthTable _table;

            void sequents_from_determinants(const Determinant<std::set<int>>& det,
                    std::set<NdSequent<std::set>>& sequents) {
                auto response_set = det.get_last(); 
                auto avoid_set = utils::set_difference(this->_all_values, response_set);
                auto avoid_set_size = avoid_set.size();
                auto all_values_size = _all_values.size();
                // fully non-deterministic cell, no sequents is necessary
                if (avoid_set_size == 0)
                    return;
                // partiality
                if (avoid_set_size == all_values_size) {
                    // make a variables-only sequent
                } else if (int att_pos; find_attitude_pos(avoid_set, att_pos)) {
                    // make a single sequent according to the attitude 
                }
            };

            NdSequent<std::set> make_variables_only_sequent() {}
            NdSequent<std::set> make_attitude_sequent() {}
            NdSequent<std::set> make_single_value_sequent() {}

            bool find_attitude_pos(const std::set<int>& avoid_set, int& pos) {
                for (size_t i {0}; i < _attitudes.size(); ++i) {
                    if (_attitudes[i].values == avoid_set) {
                        pos = i;
                        return true;
                    }
                }
                return false;
            }

    };
};

#endif
