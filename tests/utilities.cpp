#include <test.hpp>

extern "C" {
#define restrict
#include <utilities.h>
#undef restrict
}

TEST(utilities, parent_position) {
    EXPECT_EQ(::parent_position(0), 0);
    EXPECT_EQ(::parent_position(1), 0);
    EXPECT_EQ(::parent_position(2), 0);
    EXPECT_EQ(::parent_position(3), 1);
    EXPECT_EQ(::parent_position(4), 1);
    EXPECT_EQ(::parent_position(5), 2);
    EXPECT_EQ(::parent_position(9), 4);
    EXPECT_EQ(::parent_position(13), 6);
    EXPECT_EQ(::parent_position(20), 9);
    EXPECT_EQ(::parent_position(58), 28);
}

TEST(utilities, lchild_position) {
    EXPECT_EQ(::lchild_position(0), 1);
    EXPECT_EQ(::lchild_position(1), 3);
    EXPECT_EQ(::lchild_position(2), 5);
    EXPECT_EQ(::lchild_position(3), 7);
    EXPECT_EQ(::lchild_position(4), 9);
    EXPECT_EQ(::lchild_position(5), 11);
    EXPECT_EQ(::lchild_position(9), 19);
    EXPECT_EQ(::lchild_position(13), 27);
    EXPECT_EQ(::lchild_position(20), 41);
    EXPECT_EQ(::lchild_position(58), 117);
}

TEST(utilities, rchild_position) {
    EXPECT_EQ(::rchild_position(0), 2);
    EXPECT_EQ(::rchild_position(1), 4);
    EXPECT_EQ(::rchild_position(2), 6);
    EXPECT_EQ(::rchild_position(3), 8);
    EXPECT_EQ(::rchild_position(4), 10);
    EXPECT_EQ(::rchild_position(5), 12);
    EXPECT_EQ(::rchild_position(9), 20);
    EXPECT_EQ(::rchild_position(13), 28);
    EXPECT_EQ(::rchild_position(20), 42);
    EXPECT_EQ(::rchild_position(58), 118);
}
