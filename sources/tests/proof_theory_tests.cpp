#include "gtest/gtest.h"
#include "core/proof-theory/sequents.h"
#include "core/proof-theory/multconc.h"
#include "core/proof-theory/ndsequents.h"
#include "core/parser/fmla/fmla_parser.h"


namespace {

    TEST(ProofTheory, Sequents) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::SetSetSequent seq {{p, p_to_q}, {q}};    
    }


    TEST(ProofTheory, NdSequentCreation) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::NdSequent<std::set> seq ({{p, q}, {}, {p_to_q}, {}});
        std::cout << seq << std::endl;
    }

    TEST(ProofTheory, NdSequentVariables) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::NdSequent<std::set> seq ({{p, q}, {}, {p_to_q}, {}});
        auto vars = seq.collect_props();
        std::cout << seq << std::endl;
        for (const auto v : vars)
            std::cout << v->symbol() << std::endl;
    }

    TEST(ProofTheory, MultipleConclusionRules) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::MultipleConclusionRule rule ("a", ltsy::NdSequent<std::set>({{p,q},{p_to_q}})); 
    }

    TEST(ProofTheory, MultipleConclusionCalculus) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");
        auto neg_p = parser.parse("neg p");
        ltsy::MultipleConclusionRule rule1
            {"exp", ltsy::NdSequent<std::set>({{p, neg_p},{q}})}; 
        ltsy::MultipleConclusionRule rule2
            {"con_i", ltsy::NdSequent<std::set>({{p, q},{p_and_q, p_or_q}})}; 
        ltsy::MultipleConclusionRule rule3
            {"disj", ltsy::NdSequent<std::set>({{p},{p_or_q}})}; 
        ltsy::MultipleConclusionCalculus calc {{rule1, rule2}, {{0,1}}};
        calc.derive(rule3, {{p}});

    }
}
