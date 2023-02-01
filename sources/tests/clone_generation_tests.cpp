#include "gtest/gtest.h"
#include "core/common.h"
#include "core/utils.h"
#include "core/parser/fmla/fmla_parser.h"
#include "apps/clones/clone_generation.h"

namespace {

    TEST(CloneGen, GenerateCloneGodel) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto neg_p = parser.parse("neg p");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");
        auto p_to_q = parser.parse("p -> q");

        auto tt_neg = ltsy::TruthTable<std::set<int>>(3, 
                {
                    {{0},{2}},
                    {{1},{0}},
                    {{2},{0}},
        }, neg_p);
        auto tt_and = ltsy::TruthTable<std::set<int>>(3, {
            {{0,0},{0}},
            {{0,1},{0}},
            {{0,2},{0}},
            {{1,0},{0}},
            {{1,1},{1}},
            {{1,2},{1}},
            {{2,0},{0}},
            {{2,1},{1}},
            {{2,2},{2}},
        }, p_and_q);
        auto tt_or = ltsy::TruthTable<std::set<int>>(3, {
            {{0,0},{0}},
            {{0,1},{1}},
            {{0,2},{2}},
            {{1,0},{1}},
            {{1,1},{1}},
            {{1,2},{2}},
            {{2,0},{2}},
            {{2,1},{2}},
            {{2,2},{2}},
        }, p_or_q);
        auto tt_to = ltsy::TruthTable<std::set<int>>(3, {
            {{0,0},{2}},
            {{0,1},{2}},
            {{0,2},{2}},
            {{1,0},{0}},
            {{1,1},{2}},
            {{1,2},{2}},
            {{2,0},{0}},
            {{2,1},{1}},
            {{2,2},{2}},
        }, p_to_q);

        //ltsy::CloneGenerator generator {4, {tt_smile, tt_csmile, tt_frown, tt_cfrown, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_csmile, tt_frown, tt_cfrown, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_frown, tt_smile, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_csmile, tt_cfrown, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_smile, tt_csmile, tt_and, tt_or}};
        ltsy::CloneGenerator generator {3, {tt_to,tt_and,tt_or,tt_neg}};
        const auto unary_clone = generator.generate(1, {p, q});
        //for (const auto& f : unary_clone) {
        //    std::cout << *f.fmla() << std::endl;
        //    std::cout << f.print().str() << std::endl;
        //}
    }

    TEST(CloneGen, GenerateCloneDeterministicTeamFour) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("p");
        auto smile_p = parser.parse("s(p)");
        auto frown_p = parser.parse("f(p)");
        auto csmile_p = parser.parse("cs(p)");
        auto cfrown_p = parser.parse("cf(p)");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");

        auto tt_and = ltsy::TruthTable<std::set<int>>(4, {
            {{0,0},{0}},
            {{0,1},{1}},
            {{0,2},{2}},
            {{0,3},{3}},
            {{1,0},{1}},
            {{1,1},{1}},
            {{1,2},{3}},
            {{1,3},{3}},
            {{2,0},{2}},
            {{2,1},{3}},
            {{2,2},{2}},
            {{2,3},{3}},
            {{3,0},{3}},
            {{3,1},{3}},
            {{3,2},{3}},
            {{3,3},{3}},
        }, p_and_q);

        auto tt_or = ltsy::TruthTable<std::set<int>>(4, {
            {{0,0},{0}},
            {{0,1},{0}},
            {{0,2},{0}},
            {{0,3},{0}},
            {{1,0},{0}},
            {{1,1},{1}},
            {{1,2},{3}},
            {{1,3},{1}},
            {{2,0},{0}},
            {{2,1},{3}},
            {{2,2},{2}},
            {{2,3},{2}},
            {{3,0},{0}},
            {{3,1},{1}},
            {{3,2},{2}},
            {{3,3},{3}},
        }, p_or_q);

        auto tt_smile = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{1}},
                {{2},{0}},
                {{3},{0}},
            },
        smile_p);

        auto tt_frown = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{3}},
                {{2},{2}},
                {{3},{0}},
            },
        frown_p);

        auto tt_csmile = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{0}},
                {{1},{3}},
                {{2},{1}},
                {{3},{0}},
            },
        csmile_p);

        auto tt_cfrown = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{2}},
                {{2},{0}},
                {{3},{3}},
            },
        cfrown_p);

        //ltsy::CloneGenerator generator {4, {tt_smile, tt_csmile, tt_frown, tt_cfrown, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_csmile, tt_frown, tt_cfrown, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_frown, tt_smile, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_csmile, tt_cfrown, tt_and, tt_or}};
        //ltsy::CloneGenerator generator {4, {tt_smile, tt_csmile, tt_and, tt_or}};
        ltsy::CloneGenerator generator {4, {tt_smile, tt_csmile, tt_cfrown, tt_and, tt_or}};
        const auto unary_clone = generator.generate(1, {p, q});
        //for (const auto& f : unary_clone) {
        //    std::cout << *f.fmla() << std::endl;
        //    std::cout << f.print().str() << std::endl;
        //}
    }

    TEST(CloneGen, GenerateCloneDeterministicFDE) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("p");
        auto smile_p = parser.parse("s(p)");
        auto frown_p = parser.parse("f(p)");
        auto csmile_p = parser.parse("cs(p)");
        auto cfrown_p = parser.parse("cf(p)");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");

        auto tt_and = ltsy::TruthTable<std::set<int>>(4, {
            {{0,0},{0}},
            {{0,1},{1}},
            {{0,2},{2}},
            {{0,3},{3}},
            {{1,0},{1}},
            {{1,1},{1}},
            {{1,2},{3}},
            {{1,3},{3}},
            {{2,0},{2}},
            {{2,1},{3}},
            {{2,2},{2}},
            {{2,3},{3}},
            {{3,0},{3}},
            {{3,1},{3}},
            {{3,2},{3}},
            {{3,3},{3}},
        }, p_and_q);

        auto tt_or = ltsy::TruthTable<std::set<int>>(4, {
            {{0,0},{0}},
            {{0,1},{0}},
            {{0,2},{0}},
            {{0,3},{0}},
            {{1,0},{0}},
            {{1,1},{1}},
            {{1,2},{3}},
            {{1,3},{1}},
            {{2,0},{0}},
            {{2,1},{3}},
            {{2,2},{2}},
            {{2,3},{2}},
            {{3,0},{0}},
            {{3,1},{1}},
            {{3,2},{2}},
            {{3,3},{3}},
        }, p_or_q);

 
        auto tt_dmneg = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{1}},
                {{2},{2}},
                {{3},{0}},
            },
        smile_p);       

        auto tt_smile = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{1}},
                {{2},{0}},
                {{3},{0}},
            },
        smile_p);

        auto tt_frown = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{3}},
                {{2},{2}},
                {{3},{0}},
            },
        frown_p);

        auto tt_csmile = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{0}},
                {{1},{3}},
                {{2},{1}},
                {{3},{0}},
            },
        csmile_p);

        auto tt_cfrown = ltsy::TruthTable<std::set<int>>(4,
            {
                {{0},{3}},
                {{1},{2}},
                {{2},{0}},
                {{3},{3}},
            },
        cfrown_p);

        ltsy::CloneGenerator generator {4, {tt_or, tt_dmneg, tt_and}};
        const auto unary_clone = generator.generate(1, {p, q});
        //for (const auto& f : unary_clone) {
        //    std::cout << *f.fmla() << std::endl;
        //    std::cout << f.print().str() << std::endl;
        //}
    }


    TEST(CloneGen, GenerateClone) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto neg_p = parser.parse("neg p");
        auto conf_p = parser.parse("conf(p)");
        auto conv_p = parser.parse("conv(p)");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");
        auto tt_neg = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0},{1}},
                    {{1},{0}},
                    {{2},{3}},
                    {{3},{2,4}},
                    {{4},{2,4}},
        }, neg_p);
        auto tt_conf = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0},{1}},
                    {{1},{0}},
                    {{2},{3}},
                    {{3},{2,4}},
                    {{4},{2,}},
        }, conf_p);
        auto tt_conv = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0},{1}},
                    {{1},{0}},
                    {{2},{3,2}},
                    {{3},{2,4}},
                    {{4},{2,4}},
        }, conv_p);   
        ltsy::CloneGenerator generator {5, {tt_neg, tt_conv, tt_conf}};
        const auto unary_clone = generator.generate(1, {p, q});
        //for (const auto& f : unary_clone) {
        //    std::cout << *f.fmla() << std::endl;
        //    std::cout << f.print().str() << std::endl;
        //}
    }

    TEST(CloneGen, GenerateUnaryCloneMci) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto neg_p = parser.parse("neg p");
        auto conf_p = parser.parse("o(p)");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");
        auto p_to_q = parser.parse("p -> q");
        auto tt_neg = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0},{1}},
                    {{1},{0}},
                    {{2},{3}},
                    {{3},{2,4}},
                    {{4},{2,4}},
        }, neg_p);
        auto tt_conf = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0},{0}},
                    {{1},{0}},
                    {{2},{0}},
                    {{3},{0}},
                    {{4},{1}},
        }, conf_p);
        auto tt_and = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0,0},{2,4}},
                    {{1,0},{3}},
                    {{2,0},{2,4}},
                    {{3,0},{3}},
                    {{4,0},{2,4}},
                    {{0,1},{3}},
                    {{1,1},{3}},
                    {{2,1},{3}},
                    {{3,1},{3}},
                    {{4,1},{3}},
                    {{0,2},{2,4}},
                    {{1,2},{3}},
                    {{2,2},{2,4}},
                    {{3,2},{3}},
                    {{4,2},{2,4}},
                    {{0,3},{3}},
                    {{1,3},{3}},
                    {{2,3},{3}},
                    {{3,3},{3}},
                    {{4,3},{3}},
                    {{0,4},{2,4}},
                    {{1,4},{3}},
                    {{2,4},{2,4}},
                    {{3,4},{3}},
                    {{4,4},{2,4}},
        }, p_and_q);
        auto tt_or = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0,0},{2,4}},
                    {{1,0},{2,4}},
                    {{2,0},{2,4}},
                    {{3,0},{2,4}},
                    {{4,0},{2,4}},
                    {{0,1},{2,4}},
                    {{1,1},{3}},
                    {{2,1},{2,4}},
                    {{3,1},{3}},
                    {{4,1},{2,4}},
                    {{0,2},{2,4}},
                    {{1,2},{2,4}},
                    {{2,2},{2,4}},
                    {{3,2},{2,4}},
                    {{4,2},{2,4}},
                    {{0,3},{2,4}},
                    {{1,3},{3}},
                    {{2,3},{2,4}},
                    {{3,3},{3}},
                    {{4,3},{2,4}},
                    {{0,4},{2,4}},
                    {{1,4},{2,4}},
                    {{2,4},{2,4}},
                    {{3,4},{2,4}},
                    {{4,4},{2,4}},
        }, p_or_q);
        auto tt_to = ltsy::TruthTable<std::set<int>>(5, 
                {
                    {{0,0},{2,4}},
                    {{1,0},{2,4}},
                    {{2,0},{2,4}},
                    {{3,0},{2,4}},
                    {{4,0},{2,4}},
                    {{0,1},{3}},
                    {{1,1},{2,4}},
                    {{2,1},{3}},
                    {{3,1},{2,4}},
                    {{4,1},{3}},
                    {{0,2},{2,4}},
                    {{1,2},{2,4}},
                    {{2,2},{2,4}},
                    {{3,2},{2,4}},
                    {{4,2},{2,4}},
                    {{0,3},{3}},
                    {{1,3},{2,4}},
                    {{2,3},{3}},
                    {{3,3},{2,4}},
                    {{4,3},{3}},
                    {{0,4},{2,4}},
                    {{1,4},{2,4}},
                    {{2,4},{2,4}},
                    {{3,4},{2,4}},
                    {{4,4},{2,4}},
        }, p_to_q);
        ltsy::CloneGenerator generator {5, {tt_neg, tt_and, tt_or, tt_to, tt_conf}};
        std::set<int> D = {0,2,4};
        std::set<int> U = {1,3};
        const auto unary_clone = generator.generate(1, {p, q},  std::nullopt,
                std::make_pair<>([&](ltsy::NDTruthTable tt) -> bool {
                    auto A = tt.at({0});
                    auto B = tt.at({2});
                    return (ltsy::utils::is_subset(A, D) and ltsy::utils::is_subset(B, U)) or
                        (ltsy::utils::is_subset(B, D) and ltsy::utils::is_subset(A, U));
                }, 2)
        );
        //for (const auto& f : unary_clone) {
        //    std::cout << *f.fmla() << std::endl;
        //    std::cout << f.print().str() << std::endl;
        //}
    }
}

