#ifndef __APPS_FACADE__
#define __APPS_FACADE__

#include "core/common.h"
#include "tt_determination/ndsequents.h"
#include "tt_determination/ndsequents_normal_form.h"
#include "core/semantics/attitudes.h"

namespace ltsy {

    class AppsFacade {

        public:

            /* App to check soundness of a rule wrt a given
             * generalized matrix.
             * */
            std::map<std::string, std::optional<std::vector<NdSequentGenMatrixValidator<std::set>::CounterExample>>>
            sequent_rule_soundness_check_gen_matrix(
                    std::shared_ptr<GenMatrix> matrix,
                    const std::vector<int>& sequent_set_correspondence,
                    const std::vector<NdSequentRule<std::set>>& rules, 
                    bool infer_complements=true,
                    int max_counter_examples=1) const {
                NdSequentGenMatrixValidator<std::set> validator {matrix, sequent_set_correspondence, infer_complements}; 
                std::map<std::string, std::optional<std::vector<NdSequentGenMatrixValidator<std::set>::CounterExample>>>
                    result;
                for (const auto& r : rules) {
                    result[r.name()] = validator.is_rule_satisfiability_preserving(r, max_counter_examples);  
                }
                return result;
            }

            /* App function to determinize a truth table.
             * */
            NDTruthTable determinize_truth_table(
                    int number_values, 
                    std::vector<Prop> props,
                    Connective connective,
                    std::vector<CognitiveAttitude> judgements,
                    std::vector<ltsy::NdSequent<std::set>> sequents,
                    const std::optional<NDTruthTable>& start_table) const {
                NdSequentTruthTableDeterminizer determinizer {number_values, props, connective, judgements, start_table};
                determinizer.determine(sequents);
                return determinizer.table(); 
            }

            /* App to axiomatize a truth table.
             * */
            std::vector<NdSequent<std::set>> axiomatize_truth_table(
                    std::set<int> all_values,
                    Connective connective,
                    std::vector<std::shared_ptr<Prop>> props,
                    std::shared_ptr<Compound> compound,
                    std::vector<CognitiveAttitude> judgements,
                    std::shared_ptr<JudgementValueCorrespondence> judg_value_correspondence,
                    const NDTruthTable& table) {

                NdSequentsFromTruthTable axiomatizer {all_values, connective, props,
                compound, judgements, judg_value_correspondence};
                return axiomatizer.axiomatize(table); 
            }
    };

}

#endif
