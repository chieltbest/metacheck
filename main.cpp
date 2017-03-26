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
template <typename A, typename B, typename C>
using foo = typename foo_impl<(A{} > B{} && B{} > C{})>::template f<A>;

// the actual test case
template <typename A, typename B, typename C>
using foo_test = std::is_same<foo<A, B, C>, A>;

int main() {
	std::cout << mc::test_all(
	        mc::section("main",
	                    mc::test<foo_test, // the function to test, should return a bool
	                             100, // the number of times to repeat the test
	                             mc::gen::uint_<>,
	                             mc::gen::uint_<>,
	                             mc::gen::uint_<>>)); // parameter to use in
	// the test
	return 0;
}
