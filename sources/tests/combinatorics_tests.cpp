#include "gtest/gtest.h"
#include "core/combinatorics/combinations.h"
#include <iostream>
#include <memory>

namespace {

    TEST(Combinatorics, DiscretureCombinations) {

        ltsy::DiscretureCombinationGenerator gen {5};
        while (gen.has_next()) {
            auto s = *(gen.next());
        }

    }
};
