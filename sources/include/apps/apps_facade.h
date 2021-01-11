#ifndef __APPS_FACADE__
#define __APPS_FACADE__

#include "core/common.h"
#include "tt_determination/ndsequents.h"
#include "tt_determination/ndsequents_normal_form.h"
#include "core/semantics/attitudes.h"
#include "apps/pnm-axiomatization/multipleconclusion.h"

namespace ltsy {

    class AppsFacade {

        public:

            /* App to axiomatize a monadic generalized matrix
             * given a ser of separators.
             * */
            std::map<std::string, MultipleConclusionCalculus>
            monadic_gen_matrix_mult_conc_axiomatizer(
                    std::shared_ptr<GenMatrix> matrix,
                    Discriminator discriminator,
                    const std::vector<int>& sequent_set_correspondence,
                    const std::vector<std::pair<int,int>>& prem_conc_corresp,
                    bool simplify_overlap=true,
                    bool simplify_dilution=true,
                    bool simplify_subrules=true
                    ) {
               PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, matrix, sequent_set_correspondence, prem_conc_corresp}; 
               return axiomatizer.make_calculus(simplify_overlap, simplify_dilution, simplify_subrules);
            }

            /* App to check soundness of a rule wrt a given
             * generalized matrix.
             * */
            std::map<std::string, std::optional<std::vector<NdSequentGenMatrixValidator<std::set>::CounterExample>>>
            sequent_rule_soundness_check_gen_matrix(
                    std::shared_ptr<GenMatrix> matrix,
                    const std::vector<int>& sequent_set_correspondence,
                    const std::vector<NdSequentRule<std::set>>& rules, 
                    int max_counter_examples=1,
                    std::optional<progresscpp::ProgressBar> progress_bar = std::nullopt) const {
                NdSequentGenMatrixValidator<std::set> validator {matrix, sequent_set_correspondence}; 
                std::map<std::string, std::optional<std::vector<NdSequentGenMatrixValidator<std::set>::CounterExample>>>
                    result;
                for (const auto& r : rules) {
                    Signature sig = r.infer_signature();
                    result[r.name()] = validator.is_rule_satisfiability_preserving(r, sig, max_counter_examples, progress_bar);  
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
