#ifndef __JUDGE_VAL_CORR__
#define __JUDGE_VAL_CORR__

#include "core/semantics/attitudes.h"
#include <set>

namespace ltsy {

    /**
     * General correspondence between values and judgements.
     * */
    class JudgementValueCorrespondence {
        protected:

            std::map<std::string, CognitiveAttitude> _judgements;

        public:
            JudgementValueCorrespondence(const decltype(_judgements)& judgements) 
                : _judgements {judgements} { /* empty */ }

            virtual std::set<CognitiveAttitude> judgements_from_value(int) = 0;
            virtual int value_from_judgements(const std::set<CognitiveAttitude>&) = 0;

            /**
             * Produces a set of lists of judgements. Each list
             * represent a configuration of a counter-model
             * in terms of judgements.
             * */
            virtual std::vector<std::set<CognitiveAttitude>> 
                judgements_from_set_values(const std::set<int>&) = 0;
            inline CognitiveAttitude get_judgement(const std::string& name) { return _judgements[name]; }
    };

    /**
     * A factory for judgement-value correspondences.
     * */
    class JudgementValueCorrespondenceFactory {
        
        public:
            
            enum class JudgementValueCorrespondenceType {
                FOUR_BILLATICE
            };

            static std::shared_ptr<JudgementValueCorrespondence> make_judgement_value_correspondence(
                    const JudgementValueCorrespondenceType& type);

    };

    /**
     * Gives a judgement-value correspondence given
     * when they are represented in a billatice,
     * the judgements occuppying the edges of the
     * underlying graph.
     *
     * For example:
     *
     * {
     *    {
     *     0, {{1, N}, {2, Y}},
     *     1, {{0, N}, {3, nY}},
     *     2, {{0, Y}, {3, nN}},
     *     3, {{1, nY}, {2, nN}}
     *    }
     * }
     *
     * */
    class BillaticeJudgementValueCorrespondence : public JudgementValueCorrespondence {

        private:

            std::map<int, std::vector<std::pair<int, CognitiveAttitude>>> _adjs;

            std::set<CognitiveAttitude> _compute_judgs_from_induced_subgraph(const std::set<int>& vals);

        public:

            BillaticeJudgementValueCorrespondence(
                    const decltype(_judgements)& judgements, 
                    const decltype(_adjs)& adjs) : JudgementValueCorrespondence {judgements}, _adjs {adjs} {}

            std::set<CognitiveAttitude> judgements_from_value(int v) override;

            int value_from_judgements(const std::set<CognitiveAttitude>& judgs) override;

            std::vector<std::set<CognitiveAttitude>> judgements_from_set_values(const std::set<int>& vals) override;
    };
}

#endif
