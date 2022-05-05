//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <functional>
#include <memory>

namespace mc {
	namespace output {

		class test_printer_base {
		public:
			virtual ~test_printer_base() = default;

			virtual void print_compiletime_test(std::string test_name, bool success, std::string output,
			                                    bool minified) = 0;

			virtual void start_runtime_test(std::string test_name, bool compiletime_success, bool minified) = 0;

			virtual void end_runtime_test(std::string test_name, bool success) = 0;

			virtual void end_test() = 0;
		};

		class section_printer_base {
		public:
			virtual ~section_printer_base() = default;

			virtual std::unique_ptr<section_printer_base> start_section(std::string section_name,
			                                                            unsigned num_tests) = 0;

			virtual void end_section() = 0;

			virtual std::unique_ptr<test_printer_base> start_test(std::string test_name, unsigned cases,
			                                                      unsigned runtime_cases) = 0;
		};

		class printer_base : public section_printer_base {
		public:
			virtual void begin_testing(unsigned total_sections, unsigned total_tests) = 0;

			virtual void end_testing() = 0;
		};

	} // namespace output
} // namespace mc
