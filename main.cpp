//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include "test/test.hpp"

int main() {
	std::cout << mc::test_all(test_section);
	return 0;
}
