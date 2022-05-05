//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "output.hpp"
#include "../random.hpp"

#include <iostream>
#include <sstream>
#include <utility>

#include <chrono>

namespace mc {
	namespace gtest {

		class gtest_printer : public output::printer_base, public output::test_printer_base {
			std::string section_name;
			unsigned num_sections, num_tests, passed = 0;
			unsigned section_depth = 0;

		public:
			explicit gtest_printer(std::string sectionName = "", unsigned num_sections = 0, unsigned num_tests = 0,
			                       unsigned section_depth = 0)
			    : section_name{std::move(sectionName)},
			      num_sections{num_sections},
			      num_tests{num_tests},
			      section_depth{section_depth} {
			}

			~gtest_printer() override = default;

			void begin_testing(unsigned total_sections, unsigned total_tests) override {
				num_sections = total_sections;
				num_tests    = total_tests;
				std::cout << "[==========] Running " << total_tests << (total_tests == 1 ? " test" : " tests")
				          << " from " << total_sections << " test " << (total_sections == 1 ? "case." : "cases.")
				          << std::endl
				          << "Seed: " << random_seed::state << std::endl;
			}

			void end_testing() override {
				std::cout << std::endl
				          << "[----------] Global test environment tear-down" << std::endl
				          << "[==========] " << num_tests << (num_tests == 1 ? " test" : " tests") << " from "
				          << num_sections << " test " << (num_sections == 1 ? "case" : "cases") << " ran. (0 ms total)"
				          << std::endl
				          << "[  PASSED  ] " << passed << " tests." << std::endl;
			}

			std::unique_ptr<output::section_printer_base> start_section(std::string subsection_name,
			                                                            unsigned subsection_tests) override {
				return std::make_unique<gtest_printer>(section_name + (section_name.empty() ? "" : "::") +
				                                               subsection_name,
				                                       subsection_tests, 0, section_depth + 1);
			}

			void end_section() override {
				// nothing
			}

			std::unique_ptr<test_printer_base> start_test(std::string test_name, unsigned int cases,
			                                              unsigned int runtime_cases) override {
				std::string test_section_name = section_name + (section_name.empty() ? "" : "::") + test_name;
				std::cout << "[----------] " << cases + runtime_cases << " tests from " << test_section_name
				          << std::endl;
				return std::make_unique<gtest_printer>(test_section_name, 0, cases + runtime_cases, section_depth + 1);
			}

			void end_test() override {
				std::cout << "[----------] " << num_tests << " tests from " << section_name << " (0 ms total)"
				          << std::endl;
			}

			void print_compiletime_test(std::string test_name, bool success, std::string output,
			                            bool minified) override {
				std::stringstream full_test_name{};
				full_test_name << section_name << "." << test_name;
				std::cout << "[ RUN      ] " << full_test_name.str() << std::endl << output << std::endl;
				std::cout << (success ? "[       OK ] " : "[  FAILED  ] ") << full_test_name.str() << " (0 ms)"
				          << std::endl;
				if (success) {
					++passed;
				}
			}

			void start_runtime_test(std::string test_name, bool compiletime_success, bool minified) override {
				std::stringstream full_test_name{};
				full_test_name << section_name << "." << test_name;
				std::cout << "[ RUN      ] " << full_test_name.str() << std::endl;
			}

			void end_runtime_test(std::string test_name, bool success) override {
				std::stringstream full_test_name{};
				full_test_name << section_name << (section_name.empty() ? "" : ".") << test_name;
				std::cout << (success ? "[       OK ] " : "[  FAILED  ] ") << full_test_name.str() << " (0 ms)"
				          << std::endl;
				if (success) {
					++passed;
				}
			}
		};

	} // namespace gtest

} // namespace mc
