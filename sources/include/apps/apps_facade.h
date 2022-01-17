#ifndef __APPS_FACADE__
#define __APPS_FACADE__

#include "core/common.h"
#include "tt_determination/ndsequents.h"
#include "tt_determination/ndsequents_normal_form.h"
#include "core/semantics/attitudes.h"
#include "apps/pnm-axiomatization/multipleconclusion.h"
#include "apps/dualization/symmetrical_calculi_dualization.h"
#include "apps/clones/clone_generation.h"

namespace ltsy {

    class AppsFacade {

        public:

            std::pair<
                std::optional<std::set<NDTruthTable>>,
                std::optional<std::set<NDTruthTable>>>
            clone_membership(const int nvalues, const std::vector<NDTruthTable>& base, const NDTruthTable& f, 
                std::optional<int> max_depth=std::nullopt, 
                bool return_clone = false) const {
                ltsy::CloneGenerator generator {nvalues, base};

                auto make_props = [](int n) {
                    std::vector<std::shared_ptr<Formula>> props;
                    for (int i = 1; i <= n; ++i)
                        props.push_back(std::make_shared<Prop>("p"+std::to_string(i)));
                    return props;
                };

                auto membership_predicate = [&](NDTruthTable tt) { return tt == f; };
                std::vector<std::shared_ptr<Formula>> props = make_props(f.arity());

                if (not return_clone) {
                    auto results = generator.generate(f.arity(), props, max_depth, std::pair<std::function<bool(NDTruthTable)>, int> {membership_predicate, 1});
                    if (results.size() > 0) {
                        return {results, std::nullopt};
                    } else return {std::nullopt, std::nullopt};
                } else {
                    auto results = generator.generate(f.arity(), props, max_depth);
                    auto searchedtt = results.find(f);
                    if (searchedtt != results.end()) {
                        return {std::set<NDTruthTable>{*searchedtt}, results};
                    } else return {std::nullopt, results};
                }
            }

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
                    std::optional<unsigned int> simplify_subrules_deriv=std::nullopt,
                    std::optional<unsigned int> simplify_by_derivation=std::nullopt,
                    bool simplify_by_cuts=true
                    ) {
               PNMMultipleConclusionAxiomatizer axiomatizer {discriminator, matrix, sequent_set_correspondence, prem_conc_corresp}; 
               if (simplify_by_derivation or simplify_subrules_deriv)
                   return 
                   axiomatizer.make_single_calculus(simplify_overlap, simplify_dilution, 
                           simplify_subrules_deriv, simplify_by_derivation, simplify_by_cuts).group();
               else
                   return axiomatizer.make_calculus(simplify_overlap, simplify_dilution, false, simplify_by_cuts);
            }

            /* App to axiomatize a monadic generalized matrix
             * given a ser of separators.
             * */
            MultipleConclusionCalculus
            simplify_mult_conc_axiomatizer(
                    const MultipleConclusionCalculus& calculus,
                    const std::vector<std::pair<int,int>>& prem_conc_corresp,
                    const std::vector<int>& sequent_dset_correspondence,
                    bool simplify_overlap=true,
                    bool simplify_dilution=true,
                    bool simplify_by_cuts=true,
                    std::optional<unsigned int> simplify_by_subrule_deriv=std::nullopt
                    ) {
               PNMMultipleConclusionAxiomatizer axiomatizer {sequent_dset_correspondence, prem_conc_corresp}; 
               return axiomatizer.simplify_calculus(calculus, simplify_overlap, simplify_dilution, simplify_by_cuts, simplify_by_subrule_deriv);
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
