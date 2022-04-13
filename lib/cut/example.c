#include "cut.h"


TEST(testSum) {
    EXPECT_EQUALS(6 + 5, 11);
    EXPECT_FALSE(-10 + 80 == 71);
}

TEST(testSub) {
    EXPECT_EQUALS(66 - 3, 63);
    EXPECT(-5 - 5 == -10);
}

TEST(badTestSum) {
    EXPECT_EQUALS(2 + 2, 5);
    EXPECT(2 + 2 == 5);
}

