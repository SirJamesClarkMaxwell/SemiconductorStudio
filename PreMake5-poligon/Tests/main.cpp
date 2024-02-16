#include "pch.h"



TEST(SomeTest, NameForSomeTest) 
{
	EXPECT_DOUBLE_EQ(3.3, MyNumericLibrary::add(1.1, 2.2),0.001);
}