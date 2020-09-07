#include "gtest/gtest.h"
#include "core/combinatorics/combinations.h"
#include "core/common.h"
#include <iostream>
#include <memory>

namespace {

    TEST(Combinatorics, DiscretureCombinations) {

        ltsy::DiscretureCombinationGenerator gen {5};
        while (gen.has_next()) {
            auto s = *(gen.next());
            std::cout << s << std::endl;
        }

    }

    TEST(Combinatorics, DiscretureCombinationsWithk) {

        ltsy::DiscretureCombinationGenerator gen {4, 2};
        while (gen.has_next()) {
            auto s = *(gen.next());
            std::cout << s << std::endl;
        }

    }
};
