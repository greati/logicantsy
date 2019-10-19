#include "gtest/gtest.h"
#include "core/utils.h"

namespace {

    TEST(CoreUtils, TupleAndPosition) {

        {
            auto [x,y,z] = ltsy::utils::tuple_from_position<3,3>(5);
            ASSERT_EQ(x, 0); ASSERT_EQ(y, 1); ASSERT_EQ(z, 2);
        }
        {
            auto [x,y,z] = ltsy::utils::tuple_from_position<3,3>(0);
            ASSERT_EQ(x, 0); ASSERT_EQ(y, 0); ASSERT_EQ(z, 0);
        }
        {
            auto [x,y,z,w] = ltsy::utils::tuple_from_position<3,4>(5);
            ASSERT_EQ(x, 0); ASSERT_EQ(y, 0); ASSERT_EQ(z, 1); ASSERT_EQ(w,2);
        }
        {
            auto [x,y,z] = ltsy::utils::tuple_from_position<3,3>(26);
            ASSERT_EQ(x, 2); ASSERT_EQ(y, 2); ASSERT_EQ(z, 2);
        }

        {
            auto pos = ltsy::utils::position_from_tuple<3,3>({0,0,0});
            ASSERT_EQ(pos, 0);
        }

        {
            auto pos = ltsy::utils::position_from_tuple<3,3>({0,1,2});
            ASSERT_EQ(pos, 5);
        }

        {
            auto pos = ltsy::utils::position_from_tuple<3,3>({2,2,2});
            ASSERT_EQ(pos, 26);
        }

        {
            auto pos = ltsy::utils::position_from_tuple<3,4>({0, 0, 1, 2});
            ASSERT_EQ(pos, 5);
        }
    }

};
