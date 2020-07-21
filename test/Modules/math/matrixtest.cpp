#include <gtest/gtest.h>

#include "Modules/moduleslib.h"

#include "Modules/math/matrix3.h"

TEST(Math, Matrix3)
{
    using namespace Engine;

    Matrix3 identity {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    Matrix3 rot1 {
        0, 1, 0,
        0, 0, 1,
        1, 0, 0
    };

    Matrix3 rot2 {
        0, 0, 1,
        1, 0, 0,
        0, 1, 0
    };

    Matrix3 flip1 {
        0, 1, 0,
        1, 0, 0,
        0, 0, 1
    };

    Matrix3 flip2 {
        1, 0, 0,
        0, 0, 1,
        0, 1, 0
    };

    Matrix3 flip3 {
        0, 0, 1,
        0, 1, 0,
        1, 0, 0
    };

    Matrix3 ones {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    };

    ASSERT_EQ(identity * identity, identity);
    ASSERT_EQ(identity * rot1, rot1);
    ASSERT_EQ(rot1 * identity, rot1);
    ASSERT_EQ(rot1 * rot1, rot2);
    ASSERT_EQ(rot1 * rot2, identity);

    ASSERT_EQ(ones * ones, 3 * ones);

    ASSERT_EQ(flip1.Inverse(), flip1);
    ASSERT_EQ(flip2.Inverse(), flip2);
    ASSERT_EQ(flip3.Inverse(), flip3);

    ASSERT_EQ(rot1.Inverse(), rot2);
    ASSERT_EQ(rot2.Inverse(), rot1);

    ASSERT_EQ(ones.Determinant(), 0);

    ASSERT_EQ(rot1[0][1], 1);
    ASSERT_EQ(rot1[1][0], 0);
}
