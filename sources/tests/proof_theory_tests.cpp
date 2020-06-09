#include "gtest/gtest.h"
#include "core/proof-theory/sequents.h"


namespace {

    TEST(ProofTheory, Sequents) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto to = std::make_shared<ltsy::Connective>("->", 2);
        auto p_to_q = std::make_shared<ltsy::Compound>(to, std::vector<std::shared_ptr<ltsy::Formula>>{p, q});
        ltsy::SetSetSequent seq {{p, p_to_q}, {q}};    
    }

}
