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
    };

}

#endif
