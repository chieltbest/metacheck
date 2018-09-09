//          Copyright Chiel Douwes 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "metacheck.hpp"

namespace mc {
	namespace detail {
		template <typename State>
		constexpr auto push_section_result(const State state, const section_base *result)
		        -> section_temp<decltype(std::tuple_cat(state.results, std::make_tuple(result))),
		                        typename State::next_seed> {
			return {.results = std::tuple_cat(state.results, std::make_tuple(result)),
			        .name    = state.name};
		}

		// add a previously calculated result
		template <typename State, typename... Tests>
		constexpr auto test_all_func(const State state, const section_base *result,
		                             const Tests... tests)
		        -> decltype(test_all_func(push_section_result(state, result), tests...)) {
			return test_all_func(push_section_result(state, result), tests...);
		};
	} // namespace detail

#define PRECALC_SECTION(SECTION)                                                            \
	mc::detail::test_all_tuple(mc::detail::empty_section_result<FILE_RANDOM>(SECTION.name), \
	                           SECTION.tests)                                               \
	        .make_result_struct()
} // namespace mc
