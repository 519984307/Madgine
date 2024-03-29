#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/math/quaternion.h"

TEST(Math, Quaternion)
{
    using namespace Engine;

    Quaternion q1 { PI / 2, Vector3::UNIT_Y };

    Matrix3 rot1 {
        0, 0, 1,
        0, 1, 0,
        -1, 0, 0
    };

    ASSERT_TRUE(q1.toMatrix().equalsWithEpsilon(rot1));

    NormalizedVector3 dir { 1, 1, 1 };

    Quaternion dirQ = Quaternion::FromDirection(dir);

     

    ASSERT_TRUE((dirQ.toMatrix() * Vector3 { Vector3::UNIT_Z }).equalsWithEpsilon(dir));
}
