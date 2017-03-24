//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include "src/metacheck.hpp"

template<typename L>
struct create_empty;
template<template <typename...> class Seq, typename...Ts>
struct create_empty<Seq<Ts...>> {
	using f = Seq<>;
};

template <typename L, typename Result>
struct reverse_impl {
	using f = Result;
};
template <template <typename ...> class Seq, typename T, typename ...Ts, typename Result>
struct reverse_impl<Seq<T, Ts...>, Result> {
	using f = typename reverse_impl<Seq<Ts...>, mc::mpl::push_front<T, Result>>::f;
};

template<typename L>
using reverse = typename reverse_impl<L, typename create_empty<L>::f>::f;

template <typename L>
using reverse_test = std::is_same<L, reverse<reverse<L>>>;

int main() {
	std::cout << mc::test_all(
	        mc::section("main",
	                    mc::test<reverse_test, // the function to test, should return a bool
	                             100, // the number of times to repeat the test
	                             // parameters to use in the test
	                             mc::gen::list_of<mc::gen::anything/*, mc::gen::uint_<50>*/>>));
	return 0;
}
