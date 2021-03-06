//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <utility>

#include "generators.hpp"
#include "random.hpp"
#include "utility.hpp"

namespace mc {
	namespace kmpl = kvasir::mpl;

	namespace detail {
		template <template <typename...> class Func, typename seed, typename Result>
		struct call_generated_result {
			/// the result of the function being called with the generated parameters
			using result = mc::mpl::call<Func, typename Result::type>;
			/// the next seed that should be used when it is wanted to generate more numbers
			using next_seed = seed;
			/// the parameters that were used in the function call; can be any list type
			using parameters = Result;
		};

		template <template <typename...> class Func, typename Result>
		using call_generated_impl =
		        call_generated_result<Func, typename Result::next_seed, typename Result::type>;

		template <template <typename...> class Func, typename seed, typename... Params>
		using call_generated =
		        call_generated_impl<Func, typename gen::list<Params...>::template generate<seed>>;

		// virtual base class so that you can use multiple translation units

		template <template <typename...> class FuncName, unsigned failnum, unsigned tries,
		          unsigned shrinks, typename seed, typename... Params>
		struct error {
			using next_seed = seed;

			// return a failure when evaluated
			constexpr static bool value = false;

			template <typename Ostream>
			auto &&print(Ostream &&stream, std::string &suite) const {
				std::string name{func_name<FuncName>{}};

				stream << "[ RUN      ] " << suite << name << std::endl
				       << "Failure after " << (tries - failnum) << "/" << tries << " tries and "
				       << shrinks << " shrinks." << std::endl
				       << "Result: " << std::string(type_name<FuncName<Params...>>{}) << std::endl
				       << "Parameters:";
				print_all(stream, "\n\t", std::string(type_name<Params>{})...) << std::endl;
				stream << "[  FAILED  ] " << suite << name << " (0 ms)" << std::endl;
				return stream;
			}

			constexpr unsigned num_tests() const {
				return 1;
			}

			constexpr unsigned num_passed() const {
				return 0;
			}

			constexpr unsigned num_failed() const {
				return 1;
			}
		};

		template <unsigned failnum, unsigned tries, unsigned shrinks, typename Result>
		struct make_error;
		template <unsigned failnum, unsigned tries, unsigned shrinks,
		          template <typename...> class FuncName, typename seed, typename... Params>
		struct make_error<failnum, tries, shrinks,
		                  call_generated_result<FuncName, seed, gen::value::list<Params...>>> {
			using f = error<FuncName, failnum, tries, shrinks, seed, typename Params::type...>;
		};

		template <template <typename...> class FuncName, unsigned tries, typename seed>
		struct pass {
			using next_seed = seed;

			// return a success when evaluated
			constexpr static bool value = true;

			template <typename Ostream>
			auto &&print(Ostream &&stream, std::string &suite) const {
				std::string name = func_name<FuncName>{};

				stream << "[ RUN      ] " << suite << name << std::endl
				       << "[       OK ] " << suite << name << " (0 ms)" << std::endl;
				return stream;
			}

			constexpr unsigned num_tests() const {
				return 1;
			}

			constexpr unsigned num_passed() const {
				return 1;
			}

			constexpr unsigned num_failed() const {
				return 0;
			}
		};

		template <typename Params, unsigned shrinks>
		struct minify_result {
			using parameters                        = Params;
			constexpr static unsigned total_shrinks = shrinks;
		};

		template <template <typename...> class Func, unsigned shrinks = 0>
		struct minify {
			// find a test that also fails the function
			template <typename Params>
			using call_pred = kmpl::bool_<(!mpl::call<Func, typename Params::type>::value)>;

			template <typename Params>
			using f = typename kmpl::call<
			        kmpl::unpack<kmpl::find_if<kmpl::cfe<call_pred>,
			                                   kmpl::front<minify<Func, shrinks + 1>>,
			                                   kmpl::always<minify_result<Params, shrinks>>>>,
			        typename gen::shrink<Params>::type>;
		};

		enum check_state { PASS, FAIL, RECURSE };

		constexpr check_state check_select(unsigned tries, bool pass = true) {
			return pass ? (tries == 0 ? PASS : RECURSE) : FAIL;
		}

		template <check_state>
		struct check_impl;
		template <>
		struct check_impl<PASS> {
			template <template <typename...> class Func, unsigned tries, unsigned total_tries,
			          typename Result, typename... Params>
			using f = pass<Func, total_tries, typename Result::next_seed>;
		};
		template <>
		struct check_impl<FAIL> {
			template <typename MinifyResult>
			struct with_minify_result {
				template <template <typename...> class Func, unsigned tries, unsigned total_tries,
				          typename Result>
				using f = typename make_error<
				        tries, total_tries, MinifyResult::total_shrinks,
				        call_generated_result<Func, typename Result::next_seed,
				                              typename MinifyResult::parameters>>::f;
			};

			template <template <typename...> class Func, unsigned tries, unsigned total_tries,
			          typename Result, typename... Params>
			using f = typename with_minify_result<typename minify<Func>::template f<
			        typename Result::parameters>>::template f<Func, tries, total_tries, Result>;
		};
		template <>
		struct check_impl<RECURSE> {
			template <template <typename...> class Func, unsigned tries, unsigned total_tries,
			          typename Result, typename... Params>
			using f = typename check_impl<check_select(
			        tries - 1,
			        call_generated<Func, typename Result::next_seed, Params...>::result::value)>::
			        template f<Func, tries - 1, total_tries,
			                   call_generated<Func, typename Result::next_seed, Params...>,
			                   Params...>;
		};

		template <template <typename...> class Func, unsigned tries, typename seed,
		          typename... Params>
		using check = typename detail::check_impl<detail::check_select(tries)>::template f<
		        Func, tries + 1, tries,
		        detail::call_generated_result<kmpl::list, seed, gen::value::list<>>, Params...>;

		template <template <typename...> class Func, unsigned tries, typename... Params>
		struct test {};

		/// single section consisting of multiple tests
		template <typename... Tests>
		struct section {
			const char *name; // pod type constructor
			std::tuple<Tests...> tests;
		};
	} // namespace detail

	/// a single test case testing a function
	/// \tparam Func the function that is to be tested, should be convertible to bool, true if
	///             the test passed, and false if the test failed
	/// \tparam tries the number of times the function should be tried with different parameters
	/// \tparam Params the parameter generators that create the parameters to be passed into the
	///             function
	template <template <typename...> class Func, unsigned tries, typename... Params>
	constexpr detail::test<Func, tries, Params...> test{};

	template <typename... Tests>
	constexpr detail::section<Tests...> section(const char *name, const Tests... tests) {
		return {.name = name, .tests = std::make_tuple(tests...)};
	}

	namespace detail {
		struct section_base {
			const char *name;

			constexpr section_base(const char *name) : name{name} {};

			virtual unsigned num_tests() const                                              = 0;
			virtual unsigned num_failed() const                                             = 0;
			virtual unsigned num_passed() const                                             = 0;
			virtual std::ostream &print(std::ostream &stream, std::string &test_root) const = 0;
		};

		template <typename Result>
		auto &&print_result(std::ostream &stream, std::string &section, Result &result) {
			return deref(result).print(stream, section);
		}

		template <typename Results, std::size_t... Ints>
		auto &&print_all_tuple_impl(std::ostream &stream, std::string &section,
		                            const Results results, const std::index_sequence<Ints...>) {
			// return the last result from the print functions
			return std::get<sizeof...(Ints)>(std::forward_as_tuple(
			        stream, print_result(stream, section, std::get<Ints>(results))...));
		}

		template <typename Results>
		auto print_all_tuple(std::ostream &stream, std::string section, const Results results)
		        -> decltype(print_all_tuple_impl(
		                stream, section, results,
		                std::make_index_sequence<std::tuple_size<Results>::value>{})) {
			return print_all_tuple_impl(
			        stream, section, results,
			        std::make_index_sequence<std::tuple_size<Results>::value>{});
		};

		template <typename Results>
		class section_result : public section_base {
		public:
			const Results results;

			constexpr section_result(const Results results, const char *name)
			    : section_base{name}, results{results} {
			}

			//			constexpr static auto num_tests = std::tuple_size<Results>::value;
			unsigned num_tests() const override {
				return std::tuple_size<Results>::value;
			}

			unsigned num_passed() const override {
				unsigned passed = 0;
				foreach
					<void>(results, [&](auto result) { passed += deref(result).num_passed(); });
				return passed;
			}
			unsigned num_failed() const override {
				unsigned failed = 0;
				foreach
					<void>(results, [&](auto result) { failed += deref(result).num_failed(); });
				return failed;
			}

			std::ostream &print(std::ostream &stream, std::string &test_root) const override {
				std::string new_root{test_root};
				new_root.append(name).append(".");
				// replace the root name with the current section for now
				stream << "[----------] " << num_tests() << " tests from " << name << std::endl;
				print_all_tuple(stream, new_root, results);
				stream << "[----------] " << num_tests() << " tests from " << name
				       << " (0 ms total)" << std::endl;
				return stream;
			}
		};

		template <typename Results, typename Seed>
		struct section_temp {
			const Results results;
			const char *name;

			using next_seed = Seed;

			constexpr const section_result<Results> make_result_struct() const {
				return {results, name};
			}
		};

		template <typename Seed>
		constexpr section_temp<std::tuple<>, Seed> empty_section_result(const char *name) {
			return {.results = {}, .name = name};
		}

		template <typename State>
		constexpr const State test_all_func(const State state) {
			return state;
		}

		template <typename State, typename TestResult>
		constexpr auto push_test_result(const State state, TestResult result)
		        -> section_temp<decltype(std::tuple_cat(state.results,
		                                                std::tuple<TestResult>{result})),
		                        typename TestResult::next_seed> {
			return {std::tuple_cat(state.results, std::tuple<TestResult>{result}), state.name};
		}

		// test a single test
		template <typename State, template <typename...> class Func, unsigned tries,
		          typename... Params, typename... Tests>
		constexpr auto test_all_func(const State state, const test<Func, tries, Params...> test,
		                             const Tests... tests)
		        -> decltype(test_all_func(
		                push_test_result(
		                        state,
		                        detail::check<Func, tries, typename State::next_seed, Params...>{}),
		                tests...)) {
			return test_all_func(
			        push_test_result(state,
			                         check<Func, tries, typename State::next_seed, Params...>{}),
			        tests...);
		}

		template <typename State, typename SectionResult>
		constexpr auto push_section_result(const State state, const SectionResult result)
		        -> section_temp<
		                decltype(std::tuple_cat(state.results,
		                                        std::make_tuple(result.make_result_struct()))),
		                typename SectionResult::next_seed> {
			return {.results = std::tuple_cat(state.results,
			                                  std::make_tuple(result.make_result_struct())),
			        .name    = state.name};
		}

		template <typename State, typename... Tests, std::size_t... Ints>
		constexpr auto test_all_tuple_impl(const State state, const std::tuple<Tests...> tests,
		                                   const std::index_sequence<Ints...>) {
			return test_all_func(state, std::get<Ints>(tests)...);
		};

		template <typename State, typename... Tests>
		constexpr auto test_all_tuple(const State state, const std::tuple<Tests...> tests)
		        -> decltype(test_all_tuple_impl(state, tests,
		                                        std::index_sequence_for<Tests...>{})) {
			return test_all_tuple_impl(state, tests, std::index_sequence_for<Tests...>{});
		};

		// test all the items in a section
		template <typename State, typename... SectionTests, typename... Tests>
		constexpr auto test_all_func(const State state, const section<SectionTests...> section,
		                             const Tests... tests)
		        -> decltype(test_all_func(
		                push_section_result(
		                        state,
		                        test_all_tuple(empty_section_result<typename State::next_seed>(
		                                               section.name),
		                                       section.tests)),
		                tests...)) {
			return test_all_func(
			        push_section_result(
			                state, test_all_tuple(empty_section_result<typename State::next_seed>(
			                                              section.name),
			                                      section.tests)),
			        tests...);
		}

		// section_base specific push section
		template<typename State>
		constexpr auto push_section_result(const State state,
		                                   const section_base *result) -> section_temp<
			decltype(std::tuple_cat(state.results, std::make_tuple(result))),
			typename State::next_seed> {
			return {.results = std::tuple_cat(state.results,
			                                  std::make_tuple(result)), .name    = state.name};
		}

		// section_base specific test_all impl
		template<typename State, typename... Tests>
		constexpr auto test_all_func(const State state, const section_base *result,
		                             const Tests... tests) -> decltype(test_all_func(
			push_section_result(state, result), tests...)) {
			return test_all_func(push_section_result(state, result), tests...);
		};

		template <typename Result>
		struct result_printer_impl {
			const Result result;

			constexpr operator int() const {
				return result.num_failed() == 0 ? 0 : 1;
			}

			friend std::ostream &operator<<(std::ostream &stream,
			                                const result_printer_impl<Result> lhs) {
				std::size_t num_suites = std::tuple_size<
				                    std::remove_reference_t<decltype(result.results)>>::value,
				            num_tests = lhs.result.num_tests();

				stream << "[==========] Running " << num_tests
				       << (num_tests == 1 ? " test" : " tests") << " from " << num_suites
				       << " test " << (num_suites == 1 ? "case." : "cases.") << std::endl
				       << "Seed: " << random_seed::state << std::endl;
				print_all_tuple(stream, std::string(""), lhs.result.results);
				stream << std::endl
				       << "[----------] Global test environment tear-down" << std::endl
				       << "[==========] " << num_tests << (num_tests == 1 ? " test" : " tests")
				       << " from " << num_suites << " test " << (num_suites == 1 ? "case" : "cases")
				       << " ran. (0 ms total)" << std::endl
				       << "[  PASSED  ] " << lhs.result.num_passed() << " tests." << std::endl;
				return stream;
			}
		};

		template <typename Result>
		constexpr auto result_printer(const Result &&result) {
			return result_printer_impl<Result>{.result = result};
		}
	} // namespace detail

	template <typename... Tests>
	constexpr auto test_all(const Tests... tests) {
		return detail::result_printer(
		        detail::test_all_func(detail::empty_section_result<random_seed>(""), tests...)
		                .make_result_struct());
	}

#define PRECALC_SECTION(SECTION)                                                            \
    mc::detail::test_all_tuple(mc::detail::empty_section_result<FILE_RANDOM>(SECTION.name), \
                               SECTION.tests)                                               \
            .make_result_struct()
} // namespace mc
