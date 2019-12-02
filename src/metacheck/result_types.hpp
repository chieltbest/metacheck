//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <sstream>
#include "minify.hpp"
#include "output/output.hpp"
#include "utility.hpp"

namespace mc {

	class result {
	public:
		std::string name;

		result(std::string name) : name{name} {
		}

		virtual ~result() {
		}

		virtual void output(output::printer_base *printer) const = 0;
	};

	namespace detail {

		template <typename... Tests>
		class section_result : public result {
		protected:
			struct print_test {
				output::printer_base *printer;
				template <typename T>
				void operator()(T test) {
					deref(test).output(printer);
				}
			};

		public:
			std::tuple<Tests...> tests;

			section_result(std::string name, Tests... tests) : result{name}, tests{tests...} {
			}

			section_result(std::string name, const std::tuple<Tests...> &tests)
			    : result(name), tests(tests) {
			}

			void output(output::printer_base *printer) const override {
				std::shared_ptr<output::printer_base> new_section = printer->start_section(name, 0);
				print_test func{new_section.get()};
				mc::foreach<void>(tests, func);
				new_section->end_section();
			}
		};

		template <typename... Tests>
		class main_section : public section_result<Tests...> {
		public:
			main_section(Tests... tests) : section_result<Tests...>{"", tests...} {
			}

			void output(output::printer_base *printer) const override {
				printer->begin_testing(0, 0); // TODO
				typename section_result<Tests...>::print_test func{printer};
				mc::foreach<void>(this->tests, func);
				printer->end_testing();
			}
		};

		template <typename... Tests>
		constexpr main_section<Tests...> make_main_section(const Tests... tests) {
			return {tests...};
		}

		// TODO: template parameters
		template <typename... TestCases>
		class test_result : public result {

			template <typename T>
			static auto attempt_runtime(T test, std::string name, output::printer_base *printer)
			        -> decltype(bool{typename T::type{}()}, kmpl::nothing{}) {
				printer->start_runtime_test(name, T::value);
				bool success = typename T::type{}();
				printer->end_runtime_test(name, success);
				return {};
			}

			template <typename... Ts>
			static void attempt_runtime(Ts...) {
			}

			template <typename T>
			static void do_print_test(T test, std::string funcname, std::string name,
			                          output::printer_base *printer) {
				std::stringstream outstr{};
				std::string func_call =
				        std::string(type_name<typename T::params::type>{})
				                .replace(0, 17, funcname); // replace kvasir::mpl::list
				outstr << "compile " << func_call << std::endl
				       << std::endl
				       << std::string(type_name<typename T::type>{});
				printer->print_compiletime_test(name, T::value, outstr.str());
			}

			struct print_test {
				std::string name;
				output::printer_base *printer;
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

		public:
			test_result(std::string name) : result(name) {
			}

			void output(output::printer_base *printer) const override {
				auto test_section = printer->start_section(name, sizeof...(TestCases));
				using minified    = first_minified<TestCases...>;
				using minified_case = typename minified::test_case;
				do_print_test(minified_case{}, name, std::string{"minified compile"},
				              test_section.get());
				attempt_runtime(minified_case{}, std::string{"minified run"},
				                test_section.get());
				foreach
					<void>(std::tuple<TestCases...>{}, print_test{name, test_section.get()});
				test_section->end_section();
			}
		};

	} // namespace detail
} // namespace mc
