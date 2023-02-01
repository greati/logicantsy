#include "gtest/gtest.h"
#include "core/proof-theory/sequents.h"
#include "core/proof-theory/multconc.h"
#include "core/proof-theory/ndsequents.h"
#include "core/parser/fmla/fmla_parser.h"
#include "core/utils.h"


namespace {

    TEST(ProofTheory, CreateASequent) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::NdSequent<std::set> seq {{{p, p_to_q}, {q}}};    
        ASSERT_EQ(seq.dimension(), 2);
        ltsy::NdSequent<std::set> seq4 {{{},{p},{},{p_to_q}}};
        ASSERT_EQ(seq4.dimension(), 4);
    }

    TEST(ProofTheory, NdSequentVariables) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        ltsy::PropSet psets {p, q};
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto bot = std::make_shared<ltsy::Connective>("bot", 0);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        auto botf = std::make_shared<ltsy::Compound>(bot, std::vector<std::shared_ptr<ltsy::Formula>>{});
        auto bot_to_bot = std::make_shared<ltsy::Compound>(to, 
                std::vector<std::shared_ptr<ltsy::Formula>>{botf, botf});
        {
            ltsy::NdSequent<std::set> seq ({{p, q}, {}, {p_to_q}, {}});
            auto vars = seq.collect_props();
            auto eq_test = (psets == vars);
            ASSERT_TRUE(eq_test);
        }
        {
            ltsy::NdSequent<std::set> seq ({{botf}, {}, {bot_to_bot}, {}});
            auto vars = seq.collect_props();
            ASSERT_EQ(vars.size(), 0);
        }
        {
            ltsy::NdSequent<std::set> seq ({{}, {}, {}, {}});
            auto vars = seq.collect_props();
            ASSERT_EQ(vars.size(), 0);
        }
    }

    TEST(ProofTheory, MultipleConclusionRulesCreation) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::MultipleConclusionRule rule ("a", ltsy::NdSequent<std::set>({{p,q},{p_to_q}}), {{0,1}}); 
    }

    TEST(ProofTheory, SubrulesGeneratorLimitCases) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        {
            ltsy::MultipleConclusionRule rule1
                {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{}}), {{0,1}}};
            ltsy::MultipleConclusionSubrulesGenerator gen {rule1};
            std::set<ltsy::MultipleConclusionRule> expected {
                {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{}}), {{0,1}}},
            };
            std::set<ltsy::MultipleConclusionRule> all_sub_rules;
            while(gen.has_next()) {
                auto r = gen.next();
                all_sub_rules.insert(r);
            }
            ASSERT_TRUE(all_sub_rules == expected);
        }
    }

    TEST(ProofTheory, SubrulesGenerator) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto p_and_q = parser.parse("p and q");
        auto p_or_q = parser.parse("p or q");
        auto neg_p = parser.parse("neg p");
        ltsy::MultipleConclusionRule rule1
            {"exp", ltsy::NdSequent<std::set>({{p, neg_p},{q}}), {{0,1}}};
        ltsy::MultipleConclusionSubrulesGenerator gen {rule1};
        std::set<ltsy::MultipleConclusionRule> expected {
            {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},{q}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p},ltsy::FmlaSet{}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p},{q}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{neg_p},ltsy::FmlaSet{}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{neg_p},{q}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p, neg_p},{}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p, neg_p},{q}}), {{0,1}}}
        };
        std::set<ltsy::MultipleConclusionRule> all_sub_rules;
        while(gen.has_next()) {
            auto r = gen.next();
            std::cout << r.sequent().to_string() << std::endl;
            all_sub_rules.insert(r);
        }
        ASSERT_TRUE(all_sub_rules == expected);
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
        auto neg_p_or_q = parser.parse("neg(p or q)");
        auto p_or_p = parser.parse("p or p");
        auto neg_p_or_p = parser.parse("(neg p) or p");
        auto neg_p = parser.parse("neg p");
        auto neg_q = parser.parse("neg q");
        auto neg_neg_p = parser.parse("neg neg p");
        auto neg_p_and_q = parser.parse("(neg p) and q");
        auto p_and_neg_p = parser.parse("p and (neg p)");
        auto neg_p_and_neg_p = parser.parse("(neg p) and (neg p)");
        auto neg_p_and_p_or_q = parser.parse("(neg p) and (p or q)");
        auto p_or_q_and_neg_p = parser.parse("(p or q) and (neg p)");
        auto p_or_q_and_p_or_q = parser.parse("(p or q) and (p or q)");
        ltsy::MultipleConclusionRule rule1
            {"exp", ltsy::NdSequent<std::set>({{p, neg_p},{p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule2
            {"con_i", ltsy::NdSequent<std::set>({{p, q},{p_and_q}}), {{0,1}}}; 
        ltsy::MCProofSearchSequentialHeuristics calc {{rule1, rule2}, {neg_p, p_or_q}};
        std::set<ltsy::MultipleConclusionRule> expected {
            {"", ltsy::NdSequent<std::set>({{neg_p, neg_neg_p},{neg_p}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p_or_q, neg_p_or_q},{p_or_q}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{neg_p, neg_p},{neg_p_and_neg_p}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{neg_p, p_or_q},{neg_p_and_p_or_q}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p_or_q, neg_p},{p_or_q_and_neg_p}}), {{0,1}}},
            {"", ltsy::NdSequent<std::set>({{p_or_q, p_or_q},{p_or_q_and_p_or_q}}), {{0,1}}},
        };
        for (const auto& e : expected)
            std::cout << e.sequent().to_string() << std::endl;
        std::cout << "====" << std::endl;
        std::set<ltsy::MultipleConclusionRule> produced;
        while (calc.has_next()) {
            auto r = calc.select_instance();
            produced.insert(r);
        }
        for (const auto& e : produced)
            std::cout << e.sequent().to_string() << std::endl;
        ASSERT_TRUE(produced == expected);
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

    TEST(ProofTheory, MultipleConclusionCalculusTrivialities) {
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
        ltsy::MultipleConclusionRule rule5
            {"empty", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{}}), {{0,1}}}; 
        ltsy::MultipleConclusionCalculus calc {{rule2, rule3, rule5}};
       {
           auto derivation = calc.derive(rule5, {{p}});
           ASSERT_TRUE(derivation->closed);
       }
    }

    TEST(ProofTheory, NegationFragmentDerivability) {
        ltsy::BisonFmlaParser parser;
        auto p = parser.parse("p");
        auto q = parser.parse("q");
        auto neg_p = parser.parse("neg p");
        auto neg_q = parser.parse("neg q");
        auto neg_r = parser.parse("neg r");
        auto neg_neg_p = parser.parse("neg neg p");
        ltsy::MultipleConclusionRule rule1
            {"DNI", ltsy::NdSequent<std::set>({{p}, {neg_neg_p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule2
            {"DNE", ltsy::NdSequent<std::set>({{neg_neg_p}, {p}}), {{0,1}}}; 
        ltsy::MultipleConclusionRule rule3
            {"EXP", ltsy::NdSequent<std::set>({{p, neg_p}, {q}}), {{0,1}}}; 
       ltsy::MultipleConclusionCalculus calc {{rule1, rule2, rule3}};

       {
           auto derivation = calc.derive(rule1, {{p}});
           ASSERT_TRUE(derivation->closed);
       }

       {
           auto derivation = calc.derive(rule2, {{p}});
           ASSERT_TRUE(derivation->closed);
       }

       {
           auto derivation = calc.derive(rule3, {{p}});
           ASSERT_TRUE(derivation->closed);
       }

       {
	   ltsy::MultipleConclusionRule rule5
            {"empty", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{}}), {{0,1}}}; 
           auto derivation = calc.derive(rule5, {{p}});
           ASSERT_FALSE(derivation->closed);
       }

       {
	   ltsy::MultipleConclusionRule rule5
            {"empty", ltsy::NdSequent<std::set>({ltsy::FmlaSet{},ltsy::FmlaSet{p}}), {{0,1}}}; 
           auto derivation = calc.derive(rule5, {{p}});
           ASSERT_FALSE(derivation->closed);
       }

       {
	   ltsy::MultipleConclusionRule rule5
            {"empty", ltsy::NdSequent<std::set>({ltsy::FmlaSet{p},ltsy::FmlaSet{}}), {{0,1}}}; 
           auto derivation = calc.derive(rule5, {{p}});
           ASSERT_FALSE(derivation->closed);
       }

       {
          ltsy::MultipleConclusionRule rule
           {"SI", ltsy::NdSequent<std::set>({{p, neg_p}, {neg_r}}), {{0,1}}}; 
           auto derivation = calc.derive(rule, {{p}});
           std::cout << derivation->print().str() << std::endl;
           ASSERT_TRUE(derivation->closed);
       }

       {
          ltsy::MultipleConclusionRule rule
           {"O", ltsy::NdSequent<std::set>({{p, neg_p}, {neg_p}}), {{0,1}}}; 
           auto derivation = calc.derive(rule, {{p}});
           std::cout << derivation->print().str() << std::endl;
           ASSERT_TRUE(derivation->closed);
       }

       {
          ltsy::MultipleConclusionRule rule
           {"D", ltsy::NdSequent<std::set>({{p, neg_p}, {q, neg_q}}), {{0,1}}}; 
           auto derivation = calc.derive(rule, {{p}});
           std::cout << derivation->print().str() << std::endl;
           ASSERT_TRUE(derivation->closed);
       }

       {
          ltsy::MultipleConclusionRule rule
           {"EXP", ltsy::NdSequent<std::set>({{p}, {neg_p}}), {{0,1}}}; 
           auto derivation = calc.derive(rule, {{p}});
           ASSERT_FALSE(derivation->closed);
       }
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
