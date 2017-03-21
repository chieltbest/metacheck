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

		template <bool b>
		struct bool_ {
			constexpr static bool value = b;
			constexpr operator bool() const {
				return b;
			}
		};

		template <unsigned u>
		struct uint_ {
			constexpr static unsigned value = u;
			constexpr operator unsigned() const {
				return u;
			}
		};

		template <bool>
		struct conditional;
		template <>
		struct conditional<true> {
			template <typename A, typename B>
			using f = A;
		};
		template <>
		struct conditional<false> {
			template <typename A, typename B>
			using f = B;
		};

		template <unsigned N, typename T, typename... Ts>
		struct at_impl {
			using f = typename at_impl<N - 1, Ts...>::f;
		};
		template <typename T, typename... Ts>
		struct at_impl<0, T, Ts...> {
			using f = T;
		};
		template <unsigned N, typename... Ts>
		using at = typename at_impl<N, Ts...>::f;

		template <bool>
		struct fold_impl;
		template <>
		struct fold_impl<true> {
			template <template <typename, typename> class Func, typename State, typename T,
			          typename... Ts>
			using f = typename fold_impl<(sizeof...(Ts) > 0)>::template f<Func, Func<State, T>,
			                                                              Ts...>;
		};
		template <>
		struct fold_impl<false> {
			template <template <typename, typename> class Func, typename State>
			using f = State;
		};

		template <template <typename, typename> class Func, typename... Ts>
		using fold = typename fold_impl<(sizeof...(Ts) > 1)>::template f<Func, Ts...>;

		template <template <typename> class Func, typename L>
		struct transform_impl;
		template <template <typename> class Func, template <typename...> class L, typename... Ts>
		struct transform_impl<Func, L<Ts...>> {
			using f = L<Func<Ts>...>;
		};

		template <template <typename> class Func, typename L>
		using transform = typename transform_impl<Func, L>::f;

		template <template <typename, typename> class Func, typename L1, typename L2>
		struct zip_with_impl;
		template <template <typename, typename> class Func, template <typename...> class L1,
		          typename... L1s, template <typename...> class L2, typename... L2s>
		struct zip_with_impl<Func, L1<L1s...>, L2<L2s...>> {
			using f = L1<Func<L1s, L2s>...>;
		};

		template <template <typename, typename> class Func, typename L1, typename L2>
		using zip_with = typename zip_with_impl<Func, L1, L2>::f;

		template <typename Ls>
		struct join_impl;
		template <template <typename...> class L, typename Ls>
		struct join_impl<L<Ls>> {
			using f = Ls;
		};
		template <template <typename...> class L, template <typename...> class L1, typename... L1s,
		          template <typename...> class L2, typename... L2s, typename... Ls>
		struct join_impl<L<L1<L1s...>, L2<L2s...>, Ls...>> {
			using f = typename join_impl<L<L1<L1s..., L2s...>, Ls...>>::f;
		};

		template <typename L>
		using join = typename join_impl<L>::f;

		template <typename Seq>
		struct uint_sequence_for_impl;
		template <typename T, T... Is>
		struct uint_sequence_for_impl<std::integer_sequence<T, Is...>> {
			using f = list<uint_<Is>...>;
		};

		template <typename... Ts>
		using uint_sequence_for =
		        typename uint_sequence_for_impl<std::index_sequence_for<Ts...>>::f;
	}
}
