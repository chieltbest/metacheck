//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "minify.hpp"
#include "output/output.hpp"
#include "utility.hpp"

namespace mc {

	class result {
	public:
		virtual ~result() = default;

		virtual void output(output::section_printer_base *printer) const = 0;
	};

	namespace detail {
		struct section_print_test {
			output::section_printer_base *printer;

			template <typename T>
			void operator()(T test) {
				deref(test).output(printer);
			}
		};

		template <typename... Tests>
		class section_result : public result {
		public:
			std::string name;

			std::tuple<Tests...> tests;

			explicit section_result(std::string name, Tests... tests) : name{std::move(name)}, tests{tests...} {
			}

			section_result(std::string name, const std::tuple<Tests...> &tests) : name{std::move(name)}, tests{tests} {
			}

			void output(output::section_printer_base *printer) const override {
				auto new_section = printer->start_section(name, 0);
				section_print_test func{new_section.get()};
				mc::foreach<void>(tests, func);
				new_section->end_section();
			}
		};

		template <typename... Tests>
		class transparent_section_result : public result {
		public:
			std::tuple<Tests...> tests;

			explicit transparent_section_result(Tests... tests) : tests{tests...} {
			}

			explicit transparent_section_result(std::tuple<Tests...> tests) : tests{tests} {
			}

			void output(output::section_printer_base *printer) const override {
				section_print_test func{printer};
				mc::foreach<void>(tests, func);
			}
		};

		template <typename... Tests>
		class main_section {
		public:
			std::tuple<Tests...> tests;

			explicit main_section(const transparent_section_result<Tests...> sec) : tests{sec.tests} {
			}

			void output(output::printer_base *printer) const {
				printer->begin_testing(0, 0); // TODO count the total number of tests
				section_print_test func{printer};
				mc::foreach<void>(tests, func);
				printer->end_testing();
			}
		};

		template <typename... Tests>
		constexpr main_section<Tests...> make_main_section(const transparent_section_result<Tests...> sec) {
			return main_section{sec};
		}

		template <typename T>
		static auto attempt_runtime(T test, const std::string &name, output::test_printer_base *printer,
		                            bool minified = false) -> decltype(bool{typename T::type{}()}, kmpl::nothing{}) {
			printer->start_runtime_test(name, T::value, minified);
			bool success = false;
			try {
				success = typename T::type{}();
			} catch (const std::runtime_error &e) {
				std::cout << e.what() << std::endl;
			} catch (const std::exception &e) {
				std::cout << e.what() << std::endl;
			} catch (...) {
				std::cout << "An unknown exception was thrown" << std::endl;
			}
			printer->end_runtime_test(name, success);
			return {};
		}

		template <typename... Ts>
		static void attempt_runtime(Ts...) {
		}

		template <typename T>
		static void do_print_test(T test, const std::string &funcname, const std::string &name,
		                          output::test_printer_base *printer, bool minified = false) {
			std::stringstream outstr{};
			std::string func_call = std::string(type_name<typename T::params::type>{})
			                                .replace(0, 17, funcname); // replace kvasir::mpl::list
			outstr << "compile " << func_call << std::endl;
			outstr << "(unwrapped) " << std::string(type_name<typename T::params>{}).erase(0, 21);
			if (!T::value) {
				outstr << std::endl << "failed with " << std::string(type_name<T>{});
			}

			printer->print_compiletime_test(name, T::value, outstr.str(), minified);
		}

		struct test_print_test {
			std::string name;
			output::test_printer_base *printer;

			unsigned testnum = 1;

			template <typename T>
			void operator()(T test) {
				std::stringstream namestr{};
				namestr << "compile " << testnum;
				do_print_test(test, name, namestr.str(), printer);
				namestr.str("");
				namestr << "run " << testnum;
				attempt_runtime(test, namestr.str(), printer);
				++testnum;
			}
		};

		// TODO: template parameters
		template <typename... TestCases>
		class test_result : public result {
		public:
			std::string name;

			test_result(std::string name) : name{std::move(name)} {
			}

			void output(output::section_printer_base *printer) const override {
				auto test_section   = printer->start_test(name, sizeof...(TestCases) + 1, sizeof...(TestCases) + 1);
				using minified      = first_minified<TestCases...>;
				using minified_case = typename minified::testcase;

				do_print_test(minified_case{}, name, std::string{"minified compile"}, test_section.get());
				attempt_runtime(minified_case{}, std::string{"minified run"}, test_section.get());

				foreach
					<void>(std::tuple<TestCases...>{}, test_print_test{name, test_section.get()});

				test_section->end_test();
			}
		};

	} // namespace detail
} // namespace mc
