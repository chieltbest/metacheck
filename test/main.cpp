//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <metacheck/output/gtest_output.hpp>
#include "test.hpp"

int main() {
	mc::gtest::gtest_printer printer{};
	mc::testing_main(
	        &printer,
	        test /*, test_section, test_section_precalc, mc::section("sub", test_section)*/);
	return 0;
}
