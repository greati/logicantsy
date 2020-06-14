#include "gtest/gtest.h"
#include "core/common.h"
#include "core/semantics/truth_tables.h"

namespace {

    TEST(Determinants, InstantiationInt) {
        ltsy::Determinant<int> det {3, 2, 3, 2};
        std::cout << det;
    }

    TEST(Determinants, InstantiationUnordSet) {
        ltsy::Determinant<std::unordered_set<int>> det {3, 2, 3, {1,2}};
        std::cout << det;
    }

    TEST(Determinants, ExtractDetsOr) {
        auto tt_or = ltsy::TruthTable(2, 
            {
                {{0,0},0},
                {{0,1},1},
                {{1,0},1},
                {{1,1},1}
            });
        auto dets = tt_or.get_determinants();
        for (auto& d : dets)
            std::cout << d << std::endl;
    }

    TEST(Determinants, NondeterministicTT) {
        auto tt_nondet = ltsy::TruthTable<std::unordered_set<int>> {2,
            {
                {{0, 0},{0, 1}},
                {{0, 1},{0, 1}},
                {{1, 0},{0, 1}},
                {{1, 1},{0, 1}},
            }
        };
        auto dets = tt_nondet.get_determinants();
        for (auto& d : dets)
            std::cout << d << std::endl;
    }

    TEST(TruthTable, NondeterministicTT) {
        auto tt_nondet = ltsy::TruthTable<std::unordered_set<int>> {2,
            {
                {{0, 0},{0, 1}},
                {{0, 1},{0, 1}},
                {{1, 0},{0, 1}},
                {{1, 1},{0, 1}},
            }
        };
    }

    TEST(TruthTable, ConstructAndAt) {
       auto tt_or = ltsy::TruthTable(2, 
               {
                   {{0,0},0},
                   {{0,1},1},
                   {{1,0},1},
                   {{1,1},1}
               });
       ASSERT_EQ(tt_or.at(0),0);
       ASSERT_EQ(tt_or.at(1),1);
       ASSERT_EQ(tt_or.at(2),1);
       ASSERT_EQ(tt_or.at(3),1);
       ASSERT_EQ(tt_or.at({0, 0}),0);
       ASSERT_EQ(tt_or.at({1, 0}),1);
    }

    TEST(TruthTable, TruthTableGeneration) {
        ltsy::TruthTableGenerator ttgen {2, 2};
        while (ttgen.has_next()) {
            auto tt = ttgen.next();
            std::cout << (*tt) << std::endl;
        }
    }

    TEST(TruthTable, NullaryTableGen) {
        ltsy::TruthTableGenerator ttgen {2, 0};
        while (ttgen.has_next()) {
            auto tt = ttgen.next();
            std::cout << (*tt) << std::endl;
        }
    }

    TEST(TruthTable, UnaryTableGen) {
        ltsy::TruthTableGenerator ttgen {2, 1};
        while (ttgen.has_next()) {
            auto tt = ttgen.next();
            std::cout << (*tt) << std::endl;
        }
    }

};
