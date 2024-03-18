#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <function.h>

int identical_return(int ret)
{
	return ret;
}

class DivisionBy2
{
public:
	DivisionBy2() = default;

	int operator()(int dividend) const
	{
		return dividend / 2;
	}
};

TEST_CASE("default constructor", "")
{
	my::function<int(void)> func;

	REQUIRE_THROWS_AS(func(), std::runtime_error);
}

TEST_CASE("nullptr constructor", "")
{
	my::function<int(void)> func(nullptr);

	REQUIRE_THROWS_AS(func(), std::runtime_error);
}

TEST_CASE("function constructor - function initialized", "")
{
	my::function<int(int)> func(identical_return);
	REQUIRE(func(777) == 777);
}

TEST_CASE("function constructor - lambda initialized", "")
{
	my::function<int(int)> func = [=](int multiplicand) { return 2 * multiplicand; };
	REQUIRE(func(777) == 1554);
}

TEST_CASE("function constructor - functor initialized", "")
{
	DivisionBy2 divider;
	my::function<int(int)> func(divider);
	REQUIRE(func(777) == 388);
}

TEST_CASE("copy constructor", "")
{
	SECTION("function", "copy argument initialized with a function")
	{
		my::function<int(int)> func = identical_return;
		my::function<int(int)> func_cpy = func;
		REQUIRE(func(777) == 777);
		REQUIRE(func_cpy(777) == 777);
	}

	SECTION("lambda", "copy argument initialized with a lambda")
	{
		my::function<int(int)> func = [=](int multiplicand) { return 2 * multiplicand; };
		my::function<int(int)> func_cpy = func;
		REQUIRE(func(777) == 1554);
		REQUIRE(func_cpy(777) == 1554);
	}

	SECTION("functor", "copy argument initialized with a functor")
	{
		DivisionBy2 divider;
		my::function<int(int)> func = divider;
		my::function<int(int)> func_cpy = func;
		REQUIRE(func(777) == 388);
		REQUIRE(func_cpy(777) == 388);
	}
}

TEST_CASE("move constructor", "")
{
	my::function<int(int)> func = identical_return;
	my::function<int(int)> func_cpy = std::move(func);
	
	REQUIRE(func == nullptr);
	REQUIRE(func_cpy(777) == 777);
}

TEST_CASE("assignment operator", "")
{
	SECTION("function", "assign a function")
	{
		my::function<int(int)> func;

		func = identical_return;
		REQUIRE(func(777) == 777);
	}

	SECTION("lambda", "assign a lambda")
	{
		my::function<int(int)> func;

		func = [=](int multiplicand) { return 2 * multiplicand; };
		REQUIRE(func(777) == 1554);
	}

	SECTION("functor", "assign a functor")
	{
		DivisionBy2 divider;
		my::function<int(int)> func;

		func = divider;
		REQUIRE(func(777) == 388);
	}

	SECTION("function lambda functor function", "reassign different types")
	{
		DivisionBy2 divider;
		my::function<int(int)> func;

		func = identical_return;
		REQUIRE(func(777) == 777);

		func = [=](int multiplicand) { return 2 * multiplicand; };
		REQUIRE(func(777) == 1554);

		func = divider;
		REQUIRE(func(777) == 388);

		func = identical_return;
		REQUIRE(func(777) == 777);
	}

	SECTION("my::function", "my::function is assigned to my::function")
	{
		DivisionBy2 divider;
		my::function<int(int)> func1;
		my::function<int(int)> func2;

		func1 = identical_return;
		func2 = divider;

		func1 = func2;
		REQUIRE(func1(777) == 388);
		REQUIRE(func2(777) == 388);
	}
}

TEST_CASE("move assignment", "")
{
	my::function<int(int)> func = identical_return;
	my::function<int(int)> func_cpy;
	func_cpy = std::move(func);

	REQUIRE(func == nullptr);
	REQUIRE(func_cpy(777) == 777);
}

TEST_CASE("out of scope call", "")
{
	my::function<int(int)> func;

	SECTION("functor", "call a my::function initialized by functor, but is out of scope")
	{
		{
			DivisionBy2 divider;
			func = divider;
		}	
		// divider is destroyed

		REQUIRE(func(777) == 388);
	}

	SECTION("lambda", "call a my::function initialized by lambda, but in another scope")
	{
		{
			func = [=](int multiplicand) { return 2 * multiplicand; };
		}

		REQUIRE(func(777) == 1554);
	}
}

TEST_CASE("vector of FunctionWrappers", "")
{
	std::vector<my::function<int(int)>> callback_queue;
	{
		DivisionBy2 divider;

		callback_queue.push_back(identical_return);
		callback_queue.push_back(divider);
		callback_queue.push_back([=](int multiplicand) {return 2 * multiplicand; });
	}

	REQUIRE(callback_queue[0](777) == 777);
	REQUIRE(callback_queue[1](777) == 388);
	REQUIRE(callback_queue[2](777) == 1554);
}

TEST_CASE("bool operator", "")
{
	my::function<int(int)> func;
	REQUIRE(bool(func) == false);
	
	func = identical_return;
	REQUIRE(bool(func) == true);
}

TEST_CASE("nullptr comparison", "")
{
	my::function<int(int)> func;
	REQUIRE(func == nullptr);

	func = identical_return;
	REQUIRE(func != nullptr);
}

static int value = 0;
static void change_value()
{
	value = 1;
}

TEST_CASE("void(void) function", "")
{
	my::function<void(void)> func;
	
	func = change_value;
	
	REQUIRE(value == 0);
	func();
	REQUIRE(value == 1);
}

TEST_CASE("call a function with many arguments", "")
{
	my::function<double(int, double, char)> func = [](int var1, double var2, char var3) { return var1 + var2 + var3; };
	REQUIRE(func(10, 10.5, 10) == 30.5);
}

TEST_CASE("const tests", "")
{
	const DivisionBy2 divider;
	const my::function<int(int)> func = divider;
	REQUIRE(func(777) == 388);

	my::function<int(int)> func_cpy = func;
	REQUIRE(func_cpy(777) == 388);
	
	func_cpy = [](int multiplicand) { return 2 * multiplicand; };
	REQUIRE(func_cpy(777) == 1554);

	func_cpy = divider;
	REQUIRE(func_cpy(777) == 388);
}

TEST_CASE("big lambda test", "")
{
	long long a, b, c, d, e, f, g, h, k, l;
	my::function<int(int)> func = [a, b, c, d, e, f, g, h, k, l](int multiplicand) { return 2 * multiplicand; };
	REQUIRE(func(777) == 1554);

	my::function<int(int)> func_cpy = func;
	REQUIRE(func_cpy(777) == 1554);

	func_cpy = DivisionBy2();
	REQUIRE(func_cpy(777) == 388);

	func_cpy = func;
	REQUIRE(func_cpy(777) == 1554);
}