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

	int operator()(int dividend)
	{
		return dividend / 2;
	}
};

TEST_CASE("default constructor", "")
{
	FunctionWrapper<int(void)> func;

	REQUIRE_THROWS_AS(func(), std::runtime_error);
}

TEST_CASE("nullptr constructor", "")
{
	FunctionWrapper<int(void)> func(nullptr);

	REQUIRE_THROWS_AS(func(), std::runtime_error);
}

TEST_CASE("function constructor - function initialized", "")
{
	FunctionWrapper<int(int)> func(identical_return);
	REQUIRE(func(777) == 777);
}

TEST_CASE("function constructor - lambda initialized", "")
{
	FunctionWrapper<int(int)> func = [=](int multiplicand) { return 2 * multiplicand; };
	REQUIRE(func(777) == 1554);
}

TEST_CASE("function constructor - functor initialized", "")
{
	DivisionBy2 divider;
	FunctionWrapper<int(int)> func(divider);
	REQUIRE(func(777) == 388);
}

TEST_CASE("copy constructor", "")
{
	SECTION("function", "copy argument initialized with a function")
	{
		FunctionWrapper<int(int)> func = identical_return;
		FunctionWrapper<int(int)> func_cpy = func;
		REQUIRE(func(777) == 777);
		REQUIRE(func_cpy(777) == 777);
	}

	SECTION("lambda", "copy argument initialized with a lambda")
	{
		FunctionWrapper<int(int)> func = [=](int multiplicand) { return 2 * multiplicand; };
		FunctionWrapper<int(int)> func_cpy = func;
		REQUIRE(func(777) == 1554);
		REQUIRE(func_cpy(777) == 1554);
	}

	SECTION("functor", "copy argument initialized with a functor")
	{
		DivisionBy2 divider;
		FunctionWrapper<int(int)> func = divider;
		FunctionWrapper<int(int)> func_cpy = func;
		REQUIRE(func(777) == 388);
		REQUIRE(func_cpy(777) == 388);
	}
}

TEST_CASE("move constructor", "")
{
	FunctionWrapper<int(int)> func = identical_return;
	FunctionWrapper<int(int)> func_cpy = std::move(func);
	
	REQUIRE(func == nullptr);
	REQUIRE(func_cpy(777) == 777);
}

TEST_CASE("assignment operator", "")
{
	SECTION("function", "assign a function")
	{
		FunctionWrapper<int(int)> func;

		func = identical_return;
		REQUIRE(func(777) == 777);
	}

	SECTION("lambda", "assign a lambda")
	{
		FunctionWrapper<int(int)> func;

		func = [=](int multiplicand) { return 2 * multiplicand; };
		REQUIRE(func(777) == 1554);
	}

	SECTION("functor", "assign a functor")
	{
		DivisionBy2 divider;
		FunctionWrapper<int(int)> func;

		func = divider;
		REQUIRE(func(777) == 388);
	}

	SECTION("function lambda functor function", "reassign different types")
	{
		DivisionBy2 divider;
		FunctionWrapper<int(int)> func;

		func = identical_return;
		REQUIRE(func(777) == 777);

		func = [=](int multiplicand) { return 2 * multiplicand; };
		REQUIRE(func(777) == 1554);

		func = divider;
		REQUIRE(func(777) == 388);

		func = identical_return;
		REQUIRE(func(777) == 777);
	}

	SECTION("FunctionWrapper", "FunctionWrapper is assigned to FunctionWrapper")
	{
		DivisionBy2 divider;
		FunctionWrapper<int(int)> func1;
		FunctionWrapper<int(int)> func2;

		func1 = identical_return;
		func2 = divider;

		func1 = func2;
		REQUIRE(func1(777) == 388);
		REQUIRE(func2(777) == 388);
	}
}

TEST_CASE("move assignment", "")
{
	FunctionWrapper<int(int)> func = identical_return;
	FunctionWrapper<int(int)> func_cpy;
	func_cpy = std::move(func);

	REQUIRE(func == nullptr);
	REQUIRE(func_cpy(777) == 777);
}

TEST_CASE("out of scope call", "")
{
	FunctionWrapper<int(int)> func;

	SECTION("functor", "call a FunctionWrapper initialized by functor, but is out of scope")
	{
		{
			DivisionBy2 divider;
			func = divider;
		}	
		// divider is destroyed

		REQUIRE(func(777) == 388);
	}

	SECTION("lambda", "call a FunctionWrapper initialized by lambda, but in another scope")
	{
		{
			func = [=](int multiplicand) { return 2 * multiplicand; };
		}

		REQUIRE(func(777) == 1554);
	}
}

TEST_CASE("vector of FunctionWrappers", "")
{
	std::vector<FunctionWrapper<int(int)>> callback_queue;
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
	FunctionWrapper<int(int)> func;
	REQUIRE(bool(func) == false);
	
	func = identical_return;
	REQUIRE(bool(func) == true);
}

TEST_CASE("nullptr comparison", "")
{
	FunctionWrapper<int(int)> func;
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
	FunctionWrapper<void(void)> func;
	
	func = change_value;
	
	REQUIRE(value == 0);
	func();
	REQUIRE(value == 1);
}

TEST_CASE("call a function with many arguments", "")
{
	FunctionWrapper<double(int, double, char)> func = [](int var1, double var2, char var3) { return var1 + var2 + var3; };
	REQUIRE(func(10, 10.5, 10) == 30.5);
}