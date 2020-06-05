#include "gtest/gtest.h"
#include "core/syntax.h"
#include <memory>

namespace {

    TEST(Connective, ValidArity) {
        EXPECT_THROW(
            try {
                auto c = ltsy::Connective("+", -1);
            } catch (const std::invalid_argument& e) {
                EXPECT_STREQ(ltsy::NEGATIVE_ARITY_EXCEPTION.c_str(), e.what());
                throw;
            },
        std::invalid_argument);
    }


    TEST(Formula, Compound) {
        auto p = std::make_shared<ltsy::Prop>("p");
        auto q = std::make_shared<ltsy::Prop>("q");
        auto land = std::make_shared<ltsy::Connective>("&", 2);
        ltsy::Compound f {land, {p, q}};
        ltsy::FormulaPrinter printer;
        f.accept(printer);
    }

    TEST(Signature, Specification) {
        ltsy::Signature cl_sig {
            {"&", 2},
            {"|", 2},
            {"->", 2},
            {"~", 1},
            {"1", 0},
            {"0", 0},
        }; 
        ASSERT_EQ(cl_sig["~"]->arity(), 1);
        ASSERT_EQ(cl_sig["|"]->arity(), 2);
        ASSERT_EQ(cl_sig(2)["&"]->arity(), 2);
        ASSERT_EQ(cl_sig(2)["|"]->arity(), 2);
        ASSERT_EQ(cl_sig(2)["->"]->arity(), 2);
    }

};
