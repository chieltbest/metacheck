//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

constexpr auto test_section_result                = PRECALC_SECTION(precalc_test_section);
const mc::detail::section_base *test_section_base = &test_section_result;
