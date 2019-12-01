//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <memory>

#include "../result_types.hpp"

namespace mc {
	namespace output {

		class printer_base {
		public:
			virtual ~printer_base() = default;

			virtual void begin_testing(unsigned total_sections, unsigned total_tests) = 0;
			virtual void end_testing()                                                = 0;

			virtual std::unique_ptr<output::printer_base> start_section(std::string section_name,
			                                                            unsigned num_tests) = 0;
			virtual void end_section()                                                      = 0;

			virtual void print_compiletime_test(std::string test_name, bool success,
			                                    std::string output) = 0;

			virtual void start_runtime_test(std::string test_name, bool compiletime_success) = 0;
			virtual void end_runtime_test(std::string test_name, bool success)               = 0;
		};

	} // namespace output
} // namespace mc
