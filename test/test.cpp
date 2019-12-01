//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

const auto test_section_result         = mc::evaluate(mc::section("precalc", test));
const mc::result *test_section_precalc = &test_section_result;
