#ifndef __ATTITUDE_SEMANTICS__
#define __ATTITUDE_SEMANTICS__

#include "core/semantics/attitudes.h"
#include "core/semantics/judgment_values_corr.h"
#include "core/semantics/genmatrix.h"

namespace ltsy {


    /**
     * A representation of the billatice
     *   2
     *  / \
     * 0   3
     *  \ /
     *   1
     * */
    class FourBillatice : public GenMatrix, public BillaticeJudgementValueCorrespondence {
        public:
           FourBillatice() : GenMatrix{std::set<int>{0, 1, 2, 3},
                                       std::vector<std::set<int>> {{2,3}, {0,1}, {0,2}, {1,3}}},
                             BillaticeJudgementValueCorrespondence {
                                 std::map<std::string, CognitiveAttitude> {
                                    {"A",   {"A", {2, 3}}},
                                    {"NA",  {"NA", {0,1}}},
                                    {"R",   {"R", {0,2}}},
                                    {"NR",  {"NR", {1,3}}}
                                 },
                                 std::map<int, std::vector<std::pair<int, CognitiveAttitude>>> {
                                    {2, {{0, {"R", {0,2}}}, {3, {"A", {2,3}}}}},
                                    {0, {{2, {"R", {0,2}}}, {1, {"NA",{0,1}}}}},
                                    {3, {{2, {"A", {2,3}}}, {1, {"NR",{1,3}}}}},
                                    {1, {{0, {"NA",{0,1}}}, {3, {"NR",{1,3}}}}}
                                }, 4
                             } { /* empty */ }

            std::set<CognitiveAttitude> get_complementary_attitudes() const override { 
                return std::set<CognitiveAttitude> {{"A", {2, 3}},
                                                   {"NA", {0,1}}};
            }
    }; 


};

#endif
