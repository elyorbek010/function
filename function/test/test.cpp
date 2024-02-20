#include <catch2/catch_test_macros.hpp>

#include "function.h"

TEST_CASE("smth", "sm other thing")
{
	REQUIRE(function() == 1);
}