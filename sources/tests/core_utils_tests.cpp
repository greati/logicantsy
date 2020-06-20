#include "gtest/gtest.h"
#include "core/utils.h"

namespace {

    TEST(CoreUtils, SetDifference) {
        std::unordered_set<int> s1  {0, 1, 2, 3};
        std::unordered_set<int> s2  {1, 2};
        std::unordered_set<int> expdiff  {0, 3};
        auto diff = ltsy::utils::set_difference(s1, s2);
        ASSERT_EQ(diff, expdiff);
    }

    TEST(CoreUtils, TupleAndPosition) {
        {
            {
                auto t = ltsy::utils::tuple_from_position(2, 1, 0);
                ASSERT_EQ(t[0], 0);
                auto t2 = ltsy::utils::tuple_from_position(2, 1, 1);
                ASSERT_EQ(t2[0], 1);
            }
        }
        {
            auto t = ltsy::utils::tuple_from_position(3, 3, 5);
            ASSERT_EQ(t[0], 0); ASSERT_EQ(t[1], 1); ASSERT_EQ(t[2], 2);
        }
        {
            auto t = ltsy::utils::tuple_from_position(2, 3, 7);
            ASSERT_EQ(t[0], 1); ASSERT_EQ(t[1], 1); ASSERT_EQ(t[2], 1);
        }
        {
            auto p = ltsy::utils::position_from_tuple(3, 3, {0, 1, 2});
            ASSERT_EQ(p, 5);
        }
        {
            auto p = ltsy::utils::position_from_tuple(2, 2, {1, 0});
            ASSERT_EQ(p, 2);
        }
    }
};
