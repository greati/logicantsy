#include "gtest/gtest.h"
#include "core/syntax.h"

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

};
