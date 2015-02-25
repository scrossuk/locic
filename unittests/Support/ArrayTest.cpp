#include <algorithm>

#include "gtest/gtest.h"

#include <locic/Support/Array.hpp>

TEST(ArrayTest, PushBack) {
	locic::Array<int, 5> array;
	EXPECT_TRUE(array.empty());
	EXPECT_EQ(array.size(), 0);
	array.push_back(0);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 1);
	array.push_back(1);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 2);
	array.push_back(2);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 3);
	array.push_back(3);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 4);
	array.push_back(4);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 5);
	array.push_back(5);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 6);
	array.push_back(6);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 7);
	array.push_back(7);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 8);
	array.push_back(8);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 9);
	array.push_back(9);
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 10);
}

TEST(ArrayTest, PushBackAndPopBack) {
	locic::Array<int, 5> array;
	
	EXPECT_TRUE(array.empty());
	EXPECT_EQ(array.size(), 0);
	
	array.push_back(0);
	
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 1);
	
	array.pop_back();
	
	EXPECT_TRUE(array.empty());
	EXPECT_EQ(array.size(), 0);
	
	array.push_back(1);
	
	EXPECT_FALSE(array.empty());
	EXPECT_EQ(array.size(), 1);
	
	array.pop_back();
	
	EXPECT_TRUE(array.empty());
	EXPECT_EQ(array.size(), 0);
}

TEST(ArrayTest, SwitchStorage) {
	locic::Array<int, 2> array;
	
	EXPECT_TRUE(array.empty());
	EXPECT_TRUE(array.using_static_space());
	EXPECT_EQ(0, array.size());
	
	array.push_back(0);
	
	EXPECT_FALSE(array.empty());
	EXPECT_TRUE(array.using_static_space());
	EXPECT_EQ(1, array.size());
	
	array.push_back(1);
	
	EXPECT_FALSE(array.empty());
	EXPECT_TRUE(array.using_static_space());
	EXPECT_EQ(2, array.size());
	
	array.push_back(2);
	
	EXPECT_FALSE(array.empty());
	EXPECT_TRUE(array.using_dynamic_space());
	EXPECT_EQ(3, array.size());
	
	array.pop_back();
	
	EXPECT_FALSE(array.empty());
	EXPECT_TRUE(array.using_static_space());
	EXPECT_EQ(2, array.size());
	
	array.push_back(3);
	
	EXPECT_FALSE(array.empty());
	EXPECT_TRUE(array.using_dynamic_space());
	EXPECT_EQ(3, array.size());
	
	array.pop_back();
	
	EXPECT_FALSE(array.empty());
	EXPECT_TRUE(array.using_static_space());
	EXPECT_EQ(2, array.size());
}

TEST(ArrayTest, ManualReverseArray) {
	locic::Array<int, 3> array;
	array.push_back(1);
	array.push_back(2);
	array.push_back(3);
	array.push_back(4);
	array.push_back(5);
	
	EXPECT_EQ(1, array[0]);
	EXPECT_EQ(2, array[1]);
	EXPECT_EQ(3, array[2]);
	EXPECT_EQ(4, array[3]);
	EXPECT_EQ(5, array[4]);
	
	auto begin = array.begin();
	auto end = array.end();
	while (begin < end) {
		std::swap(*begin, *(end - 1));
		++begin;
		--end;
	}
	
	EXPECT_EQ(5, array[0]);
	EXPECT_EQ(4, array[1]);
	EXPECT_EQ(3, array[2]);
	EXPECT_EQ(2, array[3]);
	EXPECT_EQ(1, array[4]);
}

TEST(ArrayTest, ReverseArray) {
	locic::Array<int, 3> array;
	array.push_back(1);
	array.push_back(2);
	array.push_back(3);
	array.push_back(4);
	array.push_back(5);
	
	EXPECT_EQ(1, array[0]);
	EXPECT_EQ(2, array[1]);
	EXPECT_EQ(3, array[2]);
	EXPECT_EQ(4, array[3]);
	EXPECT_EQ(5, array[4]);
	
	std::reverse(array.begin(), array.end());
	
	EXPECT_EQ(5, array[0]);
	EXPECT_EQ(4, array[1]);
	EXPECT_EQ(3, array[2]);
	EXPECT_EQ(2, array[3]);
	EXPECT_EQ(1, array[4]);
}

TEST(ArrayTest, SortArray) {
	locic::Array<int, 2> array;
	array.push_back(3);
	array.push_back(5);
	array.push_back(2);
	array.push_back(4);
	array.push_back(1);
	
	EXPECT_EQ(3, array[0]);
	EXPECT_EQ(5, array[1]);
	EXPECT_EQ(2, array[2]);
	EXPECT_EQ(4, array[3]);
	EXPECT_EQ(1, array[4]);
	
	std::sort(array.begin(), array.end());
	
	EXPECT_EQ(1, array[0]);
	EXPECT_EQ(2, array[1]);
	EXPECT_EQ(3, array[2]);
	EXPECT_EQ(4, array[3]);
	EXPECT_EQ(5, array[4]);
}

TEST(ArrayTest, SwapStaticArrays) {
	locic::Array<int, 10> array0;
	array0.push_back(0);
	array0.push_back(1);
	array0.push_back(2);
	array0.push_back(3);
	array0.push_back(4);
	
	locic::Array<int, 10> array1;
	array1.push_back(1000);
	array1.push_back(2000);
	
	std::swap(array0, array1);
	
	EXPECT_EQ(2, array0.size());
	EXPECT_EQ(5, array1.size());
	
	EXPECT_EQ(1000, array0[0]);
	EXPECT_EQ(2000, array0[1]);
	
	EXPECT_EQ(0, array1[0]);
	EXPECT_EQ(1, array1[1]);
	EXPECT_EQ(2, array1[2]);
	EXPECT_EQ(3, array1[3]);
	EXPECT_EQ(4, array1[4]);
}

TEST(ArrayTest, SwapStaticAndDynamicArrays) {
	locic::Array<int, 3> array0;
	array0.push_back(0);
	array0.push_back(1);
	array0.push_back(2);
	array0.push_back(3);
	array0.push_back(4);
	
	locic::Array<int, 3> array1;
	array1.push_back(1000);
	array1.push_back(2000);
	
	std::swap(array0, array1);
	
	EXPECT_EQ(2, array0.size());
	EXPECT_EQ(5, array1.size());
	
	EXPECT_EQ(1000, array0[0]);
	EXPECT_EQ(2000, array0[1]);
	
	EXPECT_EQ(0, array1[0]);
	EXPECT_EQ(1, array1[1]);
	EXPECT_EQ(2, array1[2]);
	EXPECT_EQ(3, array1[3]);
	EXPECT_EQ(4, array1[4]);
}

TEST(ArrayTest, SwapDynamicArrays) {
	locic::Array<int, 1> array0;
	array0.push_back(0);
	array0.push_back(1);
	array0.push_back(2);
	array0.push_back(3);
	array0.push_back(4);
	
	locic::Array<int, 1> array1;
	array1.push_back(1000);
	array1.push_back(2000);
	
	std::swap(array0, array1);
	
	EXPECT_EQ(2, array0.size());
	EXPECT_EQ(5, array1.size());
	
	EXPECT_EQ(1000, array0[0]);
	EXPECT_EQ(2000, array0[1]);
	
	EXPECT_EQ(0, array1[0]);
	EXPECT_EQ(1, array1[1]);
	EXPECT_EQ(2, array1[2]);
	EXPECT_EQ(3, array1[3]);
	EXPECT_EQ(4, array1[4]);
}

TEST(ArrayTest, HugeArray) {
	locic::Array<int, 5> array;
	for (size_t i = 0; i < 500; i++) {
		array.push_back(static_cast<int>(i));
	}
	
	EXPECT_EQ(500, array.size());
	for (size_t i = 0; i < 500; i++) {
		EXPECT_EQ(static_cast<int>(i), array[i]);
	}
}

class MoveOnlyType {
public:
	MoveOnlyType() { }
	
	MoveOnlyType(MoveOnlyType&&) = default;
	MoveOnlyType& operator=(MoveOnlyType&&) = default;
	
	MoveOnlyType(const MoveOnlyType&) = delete;
	MoveOnlyType& operator=(const MoveOnlyType&) = delete;
};

TEST(ArrayTest, MoveElements) {
	locic::Array<MoveOnlyType, 5> array;
	array.push_back(MoveOnlyType());
	array.push_back(MoveOnlyType());
	array.push_back(MoveOnlyType());
	
	EXPECT_EQ(3, array.size());
	
	locic::Array<MoveOnlyType, 5> newArray = std::move(array);
	
	EXPECT_EQ(0, array.size());
	EXPECT_EQ(3, newArray.size());
}

TEST(ArrayTest, CopyElements) {
	locic::Array<int, 5> array;
	array.push_back(0);
	array.push_back(1);
	array.push_back(2);
	
	EXPECT_EQ(3, array.size());
	EXPECT_EQ(0, array[0]);
	EXPECT_EQ(1, array[1]);
	EXPECT_EQ(2, array[2]);
	
	const locic::Array<int, 5> newArray = array.copy();
	
	EXPECT_EQ(3, array.size());
	EXPECT_EQ(0, array[0]);
	EXPECT_EQ(1, array[1]);
	EXPECT_EQ(2, array[2]);
	
	EXPECT_EQ(3, newArray.size());
	EXPECT_EQ(0, newArray[0]);
	EXPECT_EQ(1, newArray[1]);
	EXPECT_EQ(2, newArray[2]);
}
