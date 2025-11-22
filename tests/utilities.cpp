#include <test.hpp>

extern "C" {
#define restrict
#include <utilities.h>
#undef restrict
}

TEST(POSITION, PARENTPOSITION) {
    EXPECT_EQ(::parent_position(0), 0LLU);
    EXPECT_EQ(::parent_position(1), 0LLU);
    EXPECT_EQ(::parent_position(2), 0LLU);
    EXPECT_EQ(::parent_position(3), 1LLU);
    EXPECT_EQ(::parent_position(4), 1LLU);
    EXPECT_EQ(::parent_position(5), 2LLU);
    EXPECT_EQ(::parent_position(9), 4LLU);
    EXPECT_EQ(::parent_position(13), 6LLU);
    EXPECT_EQ(::parent_position(20), 9LLU);
    EXPECT_EQ(::parent_position(58), 28LLU);
}

TEST(POSITION, LEFTCHILDPOSITION) {
    EXPECT_EQ(::lchild_position(0), 1LLU);
    EXPECT_EQ(::lchild_position(1), 3LLU);
    EXPECT_EQ(::lchild_position(2), 5LLU);
    EXPECT_EQ(::lchild_position(3), 7LLU);
    EXPECT_EQ(::lchild_position(4), 9LLU);
    EXPECT_EQ(::lchild_position(5), 11LLU);
    EXPECT_EQ(::lchild_position(9), 19LLU);
    EXPECT_EQ(::lchild_position(13), 27LLU);
    EXPECT_EQ(::lchild_position(20), 41LLU);
    EXPECT_EQ(::lchild_position(58), 117LLU);
}

TEST(POSITION, RIGHTCHILDPOSITION) {
    EXPECT_EQ(::rchild_position(0), 2LLU);
    EXPECT_EQ(::rchild_position(1), 4LLU);
    EXPECT_EQ(::rchild_position(2), 6LLU);
    EXPECT_EQ(::rchild_position(3), 8LLU);
    EXPECT_EQ(::rchild_position(4), 10LLU);
    EXPECT_EQ(::rchild_position(5), 12LLU);
    EXPECT_EQ(::rchild_position(9), 20LLU);
    EXPECT_EQ(::rchild_position(13), 28LLU);
    EXPECT_EQ(::rchild_position(20), 42LLU);
    EXPECT_EQ(::rchild_position(58), 118LLU);
}
