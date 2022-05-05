//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <tuple>
#include <utility>

#include "generators.hpp"
#include "interface.hpp"
#include "random.hpp"
#include "test_all.hpp"

#include "output/output.hpp"

namespace mc {
	/// a single test case testing a function
	/// \tparam Func the function that is to be tested, should have a constexpr static bool variable called "value",
	/// 			which is true if the test passed, and false if the test failed
	/// \tparam tries the number of times the function should be tried with different parameters
	/// \tparam Params the parameter generators that create the parameters to be passed into the
	///             function
	template <template <typename...> class Func, unsigned tries, typename... Params>
	constexpr detail::test<Func, tries, Params...> test{};

	template <typename... Tests>
	constexpr detail::section<Tests...> section(const char *name, const Tests... tests) {
		return detail::section<Tests...>{name, std::make_tuple(tests...)};
	}

	template <typename... Tests>
	constexpr detail::transparent_section<Tests...> join(const Tests... tests) {
		return detail::transparent_section<Tests...>{tests...};
	}

	template <typename... Tests>
	constexpr auto evaluate(const Tests... tests) {
		return detail::test_single<random_seed>(detail::transparent_section<Tests...>{std::tuple<Tests...>{tests...}})
		        .result;
	}

	template <typename... Tests>
	void testing_main(output::printer_base *printer, const Tests... tests) {
		return detail::make_main_section(mc::evaluate(tests...)).output(printer);
	}

} // namespace mc
