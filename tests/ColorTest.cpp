#include <gtest/gtest.h>

#include "../src/Color.h"

TEST(Color, MethodTests)
{
    EXPECT_EQ(LightColor::Red | LightColor::Green, LightColor::Yellow);
    EXPECT_EQ(LightColor::Blue | LightColor::Yellow, LightColor::White);
    EXPECT_EQ(invert(LightColor::Red), LightColor::Cyan);
    EXPECT_EQ(invert(LightColor::Blue), LightColor::Yellow);
    EXPECT_EQ(LightColor::Cyan & LightColor::Blue, LightColor::Blue);
}