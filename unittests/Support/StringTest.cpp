#include <algorithm>

#include "gtest/gtest.h"

#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

TEST(StringTest, EmptyLiteral) {
	locic::StringHost stringHost;
	locic::String string0(stringHost, "");
	
	EXPECT_TRUE(string0.empty());
	EXPECT_EQ(0, string0.size());
	EXPECT_EQ(0, string0.length());
	
	locic::String string1(stringHost, "");
	EXPECT_EQ(string0, string1);
}

TEST(StringTest, Concat) {
	locic::StringHost stringHost;
	locic::String string0(stringHost, "Hello");
	locic::String string1 = locic::String(stringHost, "Text: ") + string0 + locic::String(stringHost, " world!");
	
	EXPECT_FALSE(string1.empty());
	EXPECT_EQ(18, string1.size());
	EXPECT_EQ(18, string1.length());
	EXPECT_EQ(locic::String(stringHost, "Text: Hello world!"), string1);
}

TEST(StringTest, HugeString) {
	locic::StringHost stringHost;
	locic::String string(stringHost, "Hello!");
	std::string stdString = "Hello!";
	
	for (size_t i = 0; i < 100; i++) {
		string = string + locic::String(stringHost, " Hello!");
		stdString += " Hello!";
	}
	
	EXPECT_FALSE(string.empty());
	EXPECT_EQ(706, string.size());
	EXPECT_EQ(706, string.length());
	EXPECT_EQ(locic::String(stringHost, stdString), string);
}

TEST(StringTest, AsStdString) {
	locic::StringHost stringHost;
	locic::String string(stringHost, "Hello world!");
	std::string stdString = string.asStdString();
	
	EXPECT_FALSE(stdString.empty());
	EXPECT_EQ(12, stdString.size());
	EXPECT_EQ(12, stdString.length());
	EXPECT_EQ(locic::String(stringHost, stdString), string);
}

TEST(StringTest, Equal) {
	locic::StringHost stringHost;
	locic::String string0(stringHost, "Hello!");
	locic::String string1(stringHost, "He");
	string1 = string1 + locic::String(stringHost, "llo!");
	
	EXPECT_EQ(6, string0.length());
	EXPECT_EQ(6, string1.length());
	EXPECT_EQ(string0, string1);
}

TEST(StringTest, Substr) {
	locic::StringHost stringHost;
	locic::String string0(stringHost, "Hello world!");
	locic::String string1 = string0.substr(3, 4);
	
	EXPECT_EQ(12, string0.length());
	EXPECT_EQ(4, string1.length());
	EXPECT_EQ(locic::String(stringHost, "lo w"), string1);
}
