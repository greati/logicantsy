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
        ltsy::MultipleConclusionRule rule ("a", ltsy::NdSequent<std::set>({{p,q},{p_to_q}}), {{0,1}}); 
    }

    TEST(ProofTheory, MultipleConclusionCalculus) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");
        auto neg_p = parser.parse("neg p");
        ltsy::MultipleConclusionRule rule1
            {"exp", ltsy::NdSequent<std::set>({{p, neg_p},{q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule2
            {"con_i", ltsy::NdSequent<std::set>({{p, q},{p_and_q, p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule3
            {"disj", ltsy::NdSequent<std::set>({{p},{p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionCalculus calc {{rule1, rule2}};
        auto derivation = calc.derive(rule3, {{p}});
        if (not derivation->closed) {
            std::cout << "underivable" << std::endl;
        } else {
            std::cout << "derivable" << std::endl;
        }
        std::string s = derivation->print().str();
        std::cout << s << std::endl;
    }

    TEST(ProofTheory, ProofSearchSequentialHeuristics) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto p_and_q = parser.parse("p and q");
        auto p_and_p = parser.parse("p and p");
        auto p_or_q = parser.parse("p or q");
        auto p_or_p = parser.parse("p or p");
        auto neg_p = parser.parse("neg p");
        ltsy::MultipleConclusionRule rule1
            {"exp", ltsy::NdSequent<std::set>({{p, neg_p},{q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule2
            {"con_i", ltsy::NdSequent<std::set>({{p, q},{p_and_q, p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule3
            {"disj", ltsy::NdSequent<std::set>({{p},{p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule4
            {"idemp", ltsy::NdSequent<std::set>({{p},{p_and_p, p_or_p}}), {{0,1}}}; 
        ltsy::MCProofSearchSequentialHeuristics calc {{rule2, rule3}, {neg_p, p_or_q}};
        int c = 0;
        while (calc.has_next()) {
            c++;
            calc.select_instance();
        }
        std::cout << c << std::endl;
    }

    TEST(ProofTheory, MultipleConclusionCalculusDerivableSimple) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto p_and_q = parser.parse("p and q");
        auto p_and_p = parser.parse("p and p");
        auto p_or_q = parser.parse("p or q");
        auto p_or_p = parser.parse("p or p");
        auto neg_p = parser.parse("neg p");
        ltsy::MultipleConclusionRule rule1
            {"exp", ltsy::NdSequent<std::set>({{p, neg_p},{q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule2
            {"con_i", ltsy::NdSequent<std::set>({{p, q},{p_and_q, p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule3
            {"disj", ltsy::NdSequent<std::set>({{p},{p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule4
            {"idemp", ltsy::NdSequent<std::set>({{p},{p_and_p, p_or_p}}), {{0,1}}}; 
        ltsy::MultipleConclusionCalculus calc {{rule2, rule3}};
        auto derivation = calc.derive(rule4, {{p}});
        if (not derivation->closed) {
            std::cout << "underivable" << std::endl;
        } else {
            std::cout << "derivable" << std::endl;
        }
        std::string s = derivation->print().str();
        std::cout << s << std::endl;
    }

    TEST(ProofTheory, MultipleConclusionCalculusDerivable) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto p_and_q = parser.parse("p and q");
        auto neg_p = parser.parse("neg p");
        auto neg_q = parser.parse("neg q");
        auto neg_p_and_q = parser.parse("neg (p and q)");
        auto p_or_q = parser.parse("p or q");
        auto p_to_q = parser.parse("p -> q");
        auto circ_p = parser.parse("o(p)");
        ltsy::MultipleConclusionRule rule1
            {"r1", ltsy::NdSequent<std::set>({{p, q}, {p_and_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule2
            {"r2", ltsy::NdSequent<std::set>({{p_and_q}, {p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule3
            {"r3", ltsy::NdSequent<std::set>({{p_and_q}, {q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule4
            {"r4", ltsy::NdSequent<std::set>({{neg_p}, {neg_p_and_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule5
            {"r5", ltsy::NdSequent<std::set>({{p}, {p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule6
            {"r6", ltsy::NdSequent<std::set>({{q}, {p_or_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule7
            {"r7", ltsy::NdSequent<std::set>({{p_or_q}, {p,q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule8
            {"r8", ltsy::NdSequent<std::set>({{p, p_to_q}, {q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule9
            {"r9", ltsy::NdSequent<std::set>({{q}, {p_to_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule10
            {"r10", ltsy::NdSequent<std::set>({ltsy::FmlaSet{}, {p, p_to_q}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule11
            {"r11", ltsy::NdSequent<std::set>({ltsy::FmlaSet{}, {p, circ_p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule12
            {"r12", ltsy::NdSequent<std::set>({{p}, {neg_p, circ_p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule13
            {"r13", ltsy::NdSequent<std::set>({{p, neg_p, circ_p}, {}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule14
            {"r14", ltsy::NdSequent<std::set>({ltsy::FmlaSet{}, {p, neg_p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule15
            {"r15", ltsy::NdSequent<std::set>({{p, q, neg_q}, {neg_p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule16
            {"r16", ltsy::NdSequent<std::set>({{neg_q}, {neg_p_and_q}}), {{0,1}}}; 
       ltsy::MultipleConclusionCalculus calc {{rule1, rule2, rule3, rule4, rule5,
           rule6, rule7, rule8, rule9, rule10, rule11, rule12, rule13, rule14, rule15}};
       auto derivation = calc.derive(rule16, {{p, neg_p}});
       if (not derivation->closed) std::cout << "underivable" << std::endl;
       else std::cout << "derivable" << std::endl;
       std::string s = derivation->print().str();
       std::cout << s << std::endl;
    }
}
