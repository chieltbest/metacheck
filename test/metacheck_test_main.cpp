//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <metacheck/metacheck.hpp>

#include <metacheck/output/gtest_output.hpp>

extern const mc::result *generators_section;
extern const mc::result *interface_section;
extern const mc::result *metacheck_section;
extern const mc::result *minify_section;
extern const mc::result *random_section;
extern const mc::result *result_types_section;
extern const mc::result *test_all_section;
extern const mc::result *test_case_section;
extern const mc::result *utility_section;

int main() {
	mc::gtest::gtest_printer printer{};
	mc::testing_main(&printer/*, generators_section*//*, interface_section, metacheck_section,
	                 minify_section, random_section, result_types_section, test_all_section,
	                 test_case_section*/, utility_section);
	return 0;
}
