#include <gtest/gtest.h>
#include <second_project/waypoints.h>

TEST(ParseWaypoint, Valid) {
    Waypoint wp;
    ASSERT_TRUE(parseWaypoint("1.5,-2,0.785", wp));
    EXPECT_DOUBLE_EQ(wp.x, 1.5);
    EXPECT_DOUBLE_EQ(wp.y, -2);
    EXPECT_DOUBLE_EQ(wp.theta, 0.785);
    ASSERT_TRUE(parseWaypoint(" 1, 2 , 3\r", wp));  // spaces and Windows line endings
    EXPECT_DOUBLE_EQ(wp.y, 2);
}

TEST(ParseWaypoint, Rejected) {
    Waypoint wp;
    for (const char* line : {"", "x,y,theta", "1,2", "1,2,3,4", "1,abc,3", "1,,3", "1,2,nan", "# comment"})
        EXPECT_FALSE(parseWaypoint(line, wp)) << line;
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
