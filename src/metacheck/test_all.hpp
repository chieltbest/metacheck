//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <string>

#include "result_types.hpp"
#include "test_case.hpp"

namespace mc {
	namespace detail {

		template <typename Result, typename Seed>
		struct result_temp {
			const Result result;

			using seed = Seed;
		};

		template <typename Seed>
		result_temp<transparent_section_result<>, Seed> empty_section_result() {
			return {transparent_section_result<>{}};
		}

		/// Test a single test or section
		template <typename Seed, template <typename...> class Func, unsigned tries, typename... Params>
		auto test_single(const test<Func, tries, Params...>)
		/*-> result_temp<typename gen_test_cases<test<Func, tries, Params...>, Seed,
		                                       kmpl::cfe<test_result>>::type,
		               typename gen_test_cases<test<Func, tries, Params...>, Seed,
		                                       kmpl::cfe<test_result>>::next_seed>*/
		{
			std::string name = std::string(func_name<Func>{});

			using result = gen_test_cases<test<Func, tries, Params...>, Seed, kmpl::cfe<test_result>>;
			return result_temp<typename result::type, typename result::next_seed>{.result = {name}};
		}

		/// add a pre-calculated section
		template <typename Seed>
		auto test_single(const result *test) -> result_temp<const result *, Seed> {
			return {.result = test};
		};

		/// test all test in a section
		template <typename State>
		State test_all_func(const State state) {
			return state;
		}

		/// add an evaluated test to a section
		template <typename... Tests, typename Seed, typename TestResult, typename NewSeed>
		auto push_test_result(const result_temp<transparent_section_result<Tests...>, Seed> state,
		                      result_temp<TestResult, NewSeed> result)
		        -> result_temp<transparent_section_result<Tests..., TestResult>, NewSeed> {
			return {.result = transparent_section_result<Tests..., TestResult>{
			                std::tuple_cat(state.result.tests, std::tuple<TestResult>{result.result})}};
		}

		// recurse over all tests
		template <typename State, typename Test, typename... Tests>
		auto test_all_func(const State state, const Test test, const Tests... tests)
		        -> decltype(test_all_func(push_test_result(state, test_single<typename State::seed>(test)), tests...)) {
			return test_all_func(push_test_result(state, test_single<typename State::seed>(test)), tests...);
		}

		template <typename State, typename... Tests, std::size_t... Ints>
		auto test_all_tuple_impl(const State state, const std::tuple<Tests...> tests,
		                         const std::index_sequence<Ints...>) {
			return test_all_func(state, std::get<Ints>(tests)...);
		};

		template <typename State, typename... Tests>
		auto test_all_tuple(const State state, const std::tuple<Tests...> tests)
		        -> decltype(test_all_tuple_impl(state, tests, std::index_sequence_for<Tests...>{})) {
			return test_all_tuple_impl(state, tests, std::index_sequence_for<Tests...>{});
		};

		template <typename... Tests>
		auto to_section_result(const transparent_section_result<Tests...> section, const char *name) {
			return section_result<Tests...>{name, section.tests};
		}

		// test all the items in a section
		template <typename Seed, typename... SectionTests>
		auto test_single(const section<SectionTests...> section)
		/*-> decltype(test_all_tuple(empty_section_result<Seed>(),
		                           section.tests))*/
		{
			auto state = test_all_tuple(empty_section_result<Seed>(), section.tests);
			return result_temp<decltype(to_section_result(state.result, section.name)), typename decltype(state)::seed>{
			        .result = to_section_result(state.result, section.name)};
		}

		template <typename Seed, typename... SectionTests>
		auto test_single(const transparent_section<SectionTests...> section) {
			return test_all_tuple(empty_section_result<Seed>(), section.tests);
		}

	} // namespace detail
} // namespace mc
