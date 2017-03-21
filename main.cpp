//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include "src/metacheck.hpp"

template <bool do_fail>
struct foo_impl {
	// normal non-bug behaviour
	template <typename T>
	using f = T;
};
template <>
struct foo_impl<true> {
	// return some unexpected type
	template <typename T>
	using f = void;
};
/// function that fails to work for the number 0
template <typename test>
using foo = typename foo_impl<(test{} == 0)>::template f<test>;

// the actual test case
template <typename test>
using foo_test = std::is_same<foo<test>, test>;

int main() {
	std::cout << mc::test_all(
	        mc::section("main",
	                mc::test<foo_test, // the function to test, should return a bool
	                         100, // the number of times to repeat the test
	                         mc::gen::uint_<100>>{}, // parameter to use in the test
	                mc::test<foo_test, 100, mc::gen::uint_<100>>{}));
	return 0;
}
