#include "test.hpp"

Test(utils, toStringWithInt)
{
	int i = 42;
	std::string expected = "42";
	cr_assert(ft::toString(i) == expected);
}

Test(utils, toStringWithChar)
{
	char c = 'c';
	std::string expected = "c";
	cr_assert(ft::toString(c) == expected);
}

Test(utils, toStringWithDouble)
{
	double d = 42.42;
	std::string expected = "42.42";
	cr_assert(ft::toString(d) == expected);
}

Test(utils, toStringWithFloat)
{
	float f = 42.42;
	std::string expected = "42.42";
	cr_assert(ft::toString(f) == expected);
}

Test(utils, toStringWithBool)
{
	bool a = true;
	bool b = false;
	std::string expected_a = "true";
	std::string expected_b = "false";
	cr_assert(ft::toString(a) == expected_a);
	cr_assert(ft::toString(b) == expected_b);
}

Test(utils, trimWithSpaces)
{
	std::string str = "   Hello, World!   ";
	std::string expected = "Hello, World!";
	ft::trim(str, " ");
	cr_assert(str == expected);
}

Test(utils, trimWithTabs)
{
	std::string str = "\t\tHello, World!\t\t";
	std::string expected = "Hello, World!";
	ft::trim(str, "\t");
	cr_assert(str == expected);
}

Test(utils, trimWithNewLines)
{
	std::string str = "\n\nHello, World!\n\n";
	std::string expected = "Hello, World!";
	ft::trim(str, "\n");
	cr_assert(str == expected);
}

Test(utils, trimWithVerticalTabs)
{
	std::string str = "\v\vHello, World!\v\v";
	std::string expected = "Hello, World!";
	ft::trim(str, "\v");
	cr_assert(str == expected);
}

Test(utils, trimWithFormFeeds)
{
	std::string str = "\f\fHello, World!\f\f";
	std::string expected = "Hello, World!";
	ft::trim(str, "\f");
	cr_assert(str == expected);
}

Test(utils, trimWithCarriageReturns)
{
	std::string str = "\r\rHello, World!\r\r";
	std::string expected = "Hello, World!";
	ft::trim(str, "\r");
	cr_assert(str == expected);
}

Test(utils, trimWithAll)
{
  std::string str = "\t\n\v\f\rHello, World!\t\n\v\f\r";
  std::string expected = "Hello, World!";
  ft::trim(str, "\t\n\v\f\r");
  cr_assert(str == expected);
}

Test(utils, splitWithSpaces)
{
	std::vector<std::string> result;
	std::string str = "Hello, World!";
	std::vector<std::string> expected = {"Hello,", "World!"};
	ft::split(result, str, " ");
	cr_assert(eq(str, result[0], expected[0]));
	cr_assert(eq(str, result[1], expected[1]));
}

Test(utils, splitWithTabs)
{
	std::vector<std::string> result;
	std::string str = "Hello,\tWorld!";
	std::vector<std::string> expected = {"Hello,", "World!"};
	ft::split(result, str, "\t");
	cr_assert(eq(str, result[0], expected[0]));
	cr_assert(eq(str, result[1], expected[1]));
}

Test(utils, splitWithNewLines)
{
	std::vector<std::string> result;
	std::string str = "Hello,\nWorld!";
	std::vector<std::string> expected = {"Hello,", "World!"};
	ft::split(result, str, "\n");
	cr_assert(eq(str, result[0], expected[0]));
	cr_assert(eq(str, result[1], expected[1]));
}
