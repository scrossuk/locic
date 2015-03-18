#include <algorithm>
#include <memory>

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
	locic::Optional<std::unique_ptr<int>> optional0;
	EXPECT_FALSE(optional0);
	
	optional0 = locic::make_optional(std::unique_ptr<int>(new int(10)));
	EXPECT_TRUE(optional0);
	EXPECT_EQ(*(*optional0), 10);
	
	locic::Optional<std::unique_ptr<int>> optional1(std::move(optional0));
	EXPECT_FALSE(optional0);
	EXPECT_TRUE(optional1);
	EXPECT_EQ(*(*optional1), 10);
	
	locic::Optional<std::unique_ptr<int>> optional2(std::move(optional1));
	EXPECT_FALSE(optional0);
	EXPECT_FALSE(optional1);
	EXPECT_TRUE(optional2);
	EXPECT_EQ(*(*optional2), 10);
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

class OptionalContainer {
public:
	OptionalContainer()
	: optional_() { }
	
	OptionalContainer(const int value)
	: optional_(std::unique_ptr<int>(new int(value))) { }
	
	OptionalContainer(OptionalContainer&&) = default;
	
	OptionalContainer& operator=(OptionalContainer&&) = default;
	
	bool has() const {
		return optional_;
	}
	
	int get() const {
		return *(*optional_);
	}
	
private:
	locic::Optional<std::unique_ptr<int>> optional_;
	
};

TEST(OptionalTest, MoveOptionalInContainer) {
	OptionalContainer container0(1000);
	EXPECT_TRUE(container0.has());
	EXPECT_EQ(container0.get(), 1000);
	
	container0 = OptionalContainer(2000);
	EXPECT_TRUE(container0.has());
	EXPECT_EQ(container0.get(), 2000);
	
	auto container1 = std::move(container0);
	EXPECT_FALSE(container0.has());
	EXPECT_TRUE(container1.has());
	EXPECT_EQ(container1.get(), 2000);
	
	auto container2 = std::move(container1);
	EXPECT_FALSE(container0.has());
	EXPECT_FALSE(container1.has());
	EXPECT_TRUE(container2.has());
	EXPECT_EQ(container2.get(), 2000);
	
	container2 = OptionalContainer(3000);
	EXPECT_FALSE(container0.has());
	EXPECT_FALSE(container1.has());
	EXPECT_TRUE(container2.has());
	EXPECT_EQ(container2.get(), 3000);
	
	container2 = OptionalContainer();
	EXPECT_FALSE(container0.has());
	EXPECT_FALSE(container1.has());
	EXPECT_FALSE(container2.has());
	
	container2 = OptionalContainer(4000);
	EXPECT_FALSE(container0.has());
	EXPECT_FALSE(container1.has());
	EXPECT_TRUE(container2.has());
	EXPECT_EQ(container2.get(), 4000);
}

class SomeValue {
public:
	SomeValue(int argI)
	: i(argI) { }
	
	int i;
	
private:
	std::unique_ptr<int> j, k;
	float l;
	OptionalContainer m;
};

TEST(OptionalTest, SwapEmptyOptionals) {
	locic::Optional<SomeValue> value0 = locic::None;
	locic::Optional<SomeValue> value1 = locic::None;
	
	EXPECT_FALSE(value0);
	EXPECT_FALSE(value1);
	
	value0.swap(value1);
	
	EXPECT_FALSE(value0);
	EXPECT_FALSE(value1);
}

TEST(OptionalTest, SwapPartialLeftOptionals) {
	locic::Optional<SomeValue> value0 = locic::make_optional(SomeValue(10));
	locic::Optional<SomeValue> value1 = locic::None;
	
	EXPECT_TRUE(value0);
	EXPECT_EQ(value0->i, 10);
	EXPECT_FALSE(value1);
	
	value0.swap(value1);
	
	EXPECT_FALSE(value0);
	EXPECT_TRUE(value1);
	EXPECT_EQ(value1->i, 10);
}

TEST(OptionalTest, SwapPartialRightOptionals) {
	locic::Optional<SomeValue> value0 = locic::None;
	locic::Optional<SomeValue> value1 = locic::make_optional(SomeValue(10));
	
	EXPECT_FALSE(value0);
	EXPECT_TRUE(value1);
	EXPECT_EQ(value1->i, 10);
	
	value0.swap(value1);
	
	EXPECT_TRUE(value0);
	EXPECT_EQ(value0->i, 10);
	EXPECT_FALSE(value1);
}

TEST(OptionalTest, SwapCompleteOptionals) {
	locic::Optional<SomeValue> value0 = locic::make_optional(SomeValue(10));
	locic::Optional<SomeValue> value1 = locic::make_optional(SomeValue(20));
	
	EXPECT_TRUE(value0);
	EXPECT_EQ(value0->i, 10);
	EXPECT_TRUE(value1);
	EXPECT_EQ(value1->i, 20);
	
	value0.swap(value1);
	
	EXPECT_TRUE(value0);
	EXPECT_EQ(value0->i, 20);
	EXPECT_TRUE(value1);
	EXPECT_EQ(value1->i, 10);
}
