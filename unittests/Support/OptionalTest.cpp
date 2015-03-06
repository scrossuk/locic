#include <algorithm>

#include "gtest/gtest.h"

#include <locic/Support/Optional.hpp>

TEST(OptionalTest, EmptyOptional) {
	locic::Optional<int> optional;
	EXPECT_FALSE(optional);
}

TEST(OptionalTest, MoveEmptyOptional) {
	locic::Optional<int> optional0;
	EXPECT_FALSE(optional0);
	
	locic::Optional<int> optional1(std::move(optional0));
	EXPECT_FALSE(optional0);
	EXPECT_FALSE(optional1);
}

TEST(OptionalTest, MoveFullOptional) {
	locic::Optional<int> optional0(10);
	EXPECT_TRUE(optional0);
	EXPECT_EQ(*optional0, 10);
	
	locic::Optional<int> optional1(std::move(optional0));
	EXPECT_FALSE(optional0);
	EXPECT_TRUE(optional1);
	EXPECT_EQ(*optional1, 10);
}

TEST(OptionalTest, MultipleMoveFullOptional) {
	locic::Optional<int> optional0(10);
	EXPECT_TRUE(optional0);
	EXPECT_EQ(*optional0, 10);
	
	locic::Optional<int> optional1(std::move(optional0));
	locic::Optional<int> optional2(std::move(optional1));
	locic::Optional<int> optional3(std::move(optional2));
	locic::Optional<int> optional4(std::move(optional3));
	EXPECT_FALSE(optional0);
	EXPECT_FALSE(optional1);
	EXPECT_FALSE(optional2);
	EXPECT_FALSE(optional3);
	EXPECT_TRUE(optional4);
	EXPECT_EQ(*optional4, 10);
}

TEST(OptionalTest, SetOptional) {
	locic::Optional<int> optional;
	EXPECT_FALSE(optional);
	
	optional = locic::make_optional(10);
	EXPECT_TRUE(optional);
	EXPECT_EQ(*optional, 10);
}

TEST(OptionalTest, SetAndMoveOptional) {
	locic::Optional<int> optional0;
	EXPECT_FALSE(optional0);
	
	optional0 = locic::make_optional(10);
	EXPECT_TRUE(optional0);
	EXPECT_EQ(*optional0, 10);
	
	locic::Optional<int> optional1(std::move(optional0));
	EXPECT_FALSE(optional0);
	EXPECT_TRUE(optional1);
	EXPECT_EQ(*optional1, 10);
}

TEST(OptionalTest, CopyFullOptional) {
	locic::Optional<int> optional0(10);
	EXPECT_TRUE(optional0);
	EXPECT_EQ(*optional0, 10);
	
	locic::Optional<int> optional1 = optional0;
	EXPECT_TRUE(optional0);
	EXPECT_EQ(*optional0, 10);
	EXPECT_TRUE(optional1);
	EXPECT_EQ(*optional1, 10);
}
