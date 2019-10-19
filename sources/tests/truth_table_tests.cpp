#include "gtest/gtest.h"
#include "core/semantics/truth_tables.h"

namespace {

    TEST(TruthTable, ConstructAndAt) {
        auto tt_or = ltsy::TruthTable<2,2>({
                    {{0,0},0},
                    {{0,1},1},
                    {{1,0},1},
                    {{1,1},1}
                });
        ASSERT_EQ(tt_or.at(0),0);
        ASSERT_EQ(tt_or.at(1),1);
        ASSERT_EQ(tt_or.at(2),1);
        ASSERT_EQ(tt_or.at(3),1);
        ASSERT_EQ(tt_or.at({0,0}),0);
        ASSERT_EQ(tt_or.at({1,0}),1);
    }

};
