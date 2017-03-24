//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include "src/metacheck.hpp"
#include "src/mpl.hpp"

template <typename...Ts>
struct foo_test {
	constexpr static bool value = false;
};

int main() {
	std::cout << mc::test_all(
	        mc::section("main",
	                    mc::test<foo_test, // the function to test, should return a bool
	                             100, // the number of times to repeat the test
	                             mc::gen::list_of<mc::gen::anything, mc::gen::uint_<20>>>)); //
	// parameters to
	// use in
	// the test
	return 0;
}
