#include "gtest/gtest.h"
#include "core/proof-theory/sequents.h"
#include "core/proof-theory/ndsequents.h"


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
        ltsy::NdSequent<std::unordered_set> seq ({{p, q}, {}, {p_to_q}, {}});
        std::cout << seq << std::endl;
    }

    TEST(ProofTheory, NdSequentVariables) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::NdSequent<std::unordered_set> seq ({{p, q}, {}, {p_to_q}, {}});
        auto vars = seq.collect_props();
        std::cout << seq << std::endl;
        for (const auto v : vars)
            std::cout << v->symbol() << std::endl;
    }

}
