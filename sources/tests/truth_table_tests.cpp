#include "gtest/gtest.h"
#include "core/common.h"
#include "core/parser/fmla/fmla_parser.h"
#include "core/semantics/truth_tables.h"

namespace {

    TEST(Determinants, InstantiationInt) {
        ltsy::Determinant<int> det {3, 2, 3, 2};
        //std::cout << det;
        std::set<int> kkk {1, 2, 3};
    }

    TEST(Determinants, InstantiationUnordSet) {
        ltsy::Determinant<std::set<int>> det {3, 2, 3, {1,2}};
        //std::cout << det;
    }

    TEST(Determinants, ExtractDetsOr) {
        auto tt_or = ltsy::TruthTable<int>(2, 
            {
                {{0,0},0},
                {{0,1},1},
                {{1,0},1},
                {{1,1},1}
            });
        auto dets = tt_or.get_determinants();
        //for (auto& d : dets)
        //    std::cout << d << std::endl;
    }

    TEST(Determinants, NondeterministicTT) {
        auto tt_nondet = ltsy::TruthTable<std::set<int>> {2,
            {
                {{0, 0},{0, 1}},
                {{0, 1},{0, 1}},
                {{1, 0},{0, 1}},
                {{1, 1},{0, 1}},
            }
        };
        auto dets = tt_nondet.get_determinants();
        //for (auto& d : dets)
        //    std::cout << d << std::endl;
    }

    TEST(TruthTable, NondeterministicTT) {
        auto tt_nondet = ltsy::TruthTable<std::set<int>> {2,
            {
                {{0, 0},{0, 1}},
                {{0, 1},{0, 1}},
                {{1, 0},{0, 1}},
                {{1, 1},{0, 1}},
            }
        };
    }

    TEST(TruthTable, ConstructAndAt) {
       auto tt_or = ltsy::TruthTable<int>(2, 
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

    TEST(TruthTable, ComposeND) {
       ltsy::BisonFmlaParser parser;
       auto neg_p = parser.parse("neg p");
       auto p_and_q = parser.parse("p and q");
       auto p_or_q = parser.parse("p or q");
       auto tt_g = ltsy::TruthTable<std::set<int>>(2, 
               {
                   {{0},{0,1}},
                   {{1},{0,1}},
       }, neg_p);
       auto tt_f = ltsy::TruthTable<std::set<int>>(2, 
               {
                   {{0,0},{0}},
                   {{1,1},{0}},
                   {{0,1},{1}},
                   {{1,0},{1}}
       }, p_and_q);
       auto comp_not_and = tt_f.compose({tt_g, tt_g});
       std::cout << *comp_not_and.fmla() << std::endl;
       std::cout << comp_not_and.print().str() << std::endl;
       auto tt_neg = ltsy::TruthTable<std::set<int>>(5, 
               {
                   {{0},{1}},
                   {{1},{0}},
                   {{2},{3}},
                   {{3},{2,4}},
                   {{4},{2,4}},
       }, neg_p);
       std::cout << std::endl;
       auto comp_not_not = tt_neg.compose({tt_neg});
       std::cout << comp_not_not.print().str() << std::endl;
    }

    TEST(TruthTable, Compose) {
       ltsy::BisonFmlaParser parser;
       auto neg_p = parser.parse("neg p");
       auto p_and_q = parser.parse("p and q");
       auto p_or_q = parser.parse("p or q");
       auto tt_not = ltsy::TruthTable<std::set<int>>(2, 
               {
                   {{0},{1}},
                   {{1},{0}},
       }, neg_p);
       auto tt_or = ltsy::TruthTable<std::set<int>>(2, 
               {
                   {{0,0},{0}},
                   {{0,1},{1}},
                   {{1,0},{1}},
                   {{1,1},{1}}
       }, p_or_q);
       auto tt_and = ltsy::TruthTable<std::set<int>>(2, 
               {
                   {{0,0},{0}},
                   {{0,1},{0}},
                   {{1,0},{0}},
                   {{1,1},{1}}
       }, p_and_q);
       auto comp_not_or = tt_not.compose({tt_or});
       std::cout << *comp_not_or.fmla() << std::endl;
       std::cout << comp_not_or.print().str() << std::endl;
       std::cout << endl;
       auto comp_not_and = tt_not.compose({tt_and});
       std::cout << *comp_not_and.fmla() << std::endl;
       std::cout << comp_not_and.print().str() << std::endl;
       auto comp_not_not = tt_not.compose({tt_not});
       std::cout << *comp_not_not.fmla() << std::endl;
       std::cout << comp_not_not.print().str() << std::endl;
    }

    TEST(TruthTable, TruthTableGeneration) {
        ltsy::TruthTableGenerator ttgen {2, 2};
        while (ttgen.has_next()) {
            auto tt = ttgen.next();
            //std::cout << (*tt) << std::endl;
        }
    }

    TEST(TruthTable, NullaryTableGen) {
        ltsy::TruthTableGenerator ttgen {2, 0};
        while (ttgen.has_next()) {
            auto tt = ttgen.next();
            //std::cout << (*tt) << std::endl;
        }
    }

    TEST(TruthTable, UnaryTableGen) {
        ltsy::TruthTableGenerator ttgen {2, 1};
        while (ttgen.has_next()) {
            auto tt = ttgen.next();
            //std::cout << (*tt) << std::endl;
        }
    }

};
