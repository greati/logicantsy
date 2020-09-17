#ifndef __APPS_FACADE__
#define __APPS_FACADE__

#include "core/common.h"
#include "tt_determination/ndsequents.h"
#include "tt_determination/ndsequents_normal_form.h"
#include "core/semantics/attitudes.h"

namespace ltsy {

    class AppsFacade {

        public:

            NDTruthTable determinize_truth_table(
                    int number_values, 
                    std::vector<Prop> props,
                    Connective connective,
                    std::vector<CognitiveAttitude> judgements,
                    std::vector<ltsy::NdSequent<std::set>> sequents) const {
                NdSequentTruthTableDeterminizer determinizer {number_values, props, connective, judgements};
                determinizer.determine(sequents);
                return determinizer.table(); 
            }

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
