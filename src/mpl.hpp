//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

namespace mc {
	namespace mpl {
		template <typename...>
		struct list {};

		template <typename E, typename List>
		struct push_front_impl;
		template <typename E, template <typename...> class Seq, typename... Ts>
		struct push_front_impl<E, Seq<Ts...>> {
			using f = Seq<E, Ts...>;
		};
		template <typename E, typename List>
		using push_front = typename push_front_impl<E, List>::f;

		template <typename T>
		struct always {
			template <typename...>
			using f = T;
		};

		template <typename...>
		constexpr bool always_false = false;

		template<bool b>
		struct bool_ {
			constexpr static bool value = b;
		};

		template<unsigned u>
		struct uint_ {
			constexpr static unsigned value = u;
		};


	}
}
