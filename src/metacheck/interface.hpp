//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

namespace mc {
	namespace detail {

		template <typename... Tests>
		struct transparent_section {
			std::tuple<Tests...> tests;
		};

		template <typename... Tests>
		struct section {
			const char *name;
			std::tuple<Tests...> tests;
		};

		template <template <typename...> class Func, unsigned tries, typename... Params>
		struct test {};

	} // namespace detail
} // namespace mc
