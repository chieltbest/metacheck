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

		template <template <typename...> class Func, typename Args>
		struct call_impl;
		template <template <typename...> class Func, template <typename...> class Seq,
		          typename... Ts>
		struct call_impl<Func, Seq<Ts...>> {
			using f = Func<Ts...>;
		};
		template <template <typename...> class Func, typename Args>
		using call = typename call_impl<Func, Args>::f;

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

		template <template <typename...> class Func, typename State, typename... Ts>
		struct fold_right_impl {
			using f = State;
		};
		template <template <typename...> class Func, typename State, typename T, typename... Ts>
		struct fold_right_impl<Func, State, T, Ts...> {
			using f = Func<typename fold_right_impl<Func, State, Ts...>::f, T>;
		};

		template <template <typename...> class Func, typename... Ts>
		using fold_right = typename fold_right_impl<Func, Ts...>::f;

		template <template <typename...> class Func, typename L>
		struct transform_impl;
		template <template <typename...> class Func, template <typename...> class L, typename... Ts>
		struct transform_impl<Func, L<Ts...>> {
			using f = L<Func<Ts>...>;
		};

		template <template <typename...> class Func, typename L>
		using transform = typename transform_impl<Func, L>::f;

		template <template <typename...> class Func, typename L1, typename L2>
		struct zip_with_impl;
		template <template <typename...> class Func, template <typename...> class L1,
		          typename... L1s, template <typename...> class L2, typename... L2s>
		struct zip_with_impl<Func, L1<L1s...>, L2<L2s...>> {
			using f = L1<Func<L1s, L2s>...>;
		};

		template <template <typename, typename> class Func, typename L1, typename L2>
		using zip_with = typename zip_with_impl<Func, L1, L2>::f;

		template <typename Ls>
		struct join_impl {
			using f = Ls;
		};
		template <template <typename...> class L, typename Ls>
		struct join_impl<L<Ls>> {
			using f = Ls;
		};
		template <template <typename...> class L, template <typename...> class L1, typename... L1s,
		          template <typename...> class L2, typename... L2s, typename... Ls>
		struct join_impl<L<L1<L1s...>, L2<L2s...>, Ls...>> {
			using f = typename join_impl<L<L<L1s..., L2s...>, Ls...>>::f;
		};

		template <typename L>
		using join = typename join_impl<L>::f;

		template <typename Seq>
		struct uint_sequence_impl;
		template <typename T, T... Is>
		struct uint_sequence_impl<std::integer_sequence<T, Is...>> {
			using f = list<uint_<Is>...>;
		};

		template <typename... Ts>
		using uint_sequence_for = typename uint_sequence_impl<std::index_sequence_for<Ts...>>::f;

		template <unsigned N>
		using uint_sequence = typename uint_sequence_impl<std::make_index_sequence<N>>::f;

		template <unsigned N, typename T>
		using repeat = transform<always<T>::template f, uint_sequence<N>>;

		enum find_if_state { FOUND, NOT_FOUND, RECURSE };
		constexpr find_if_state find_if_selector(bool found, int left) {
			return found ? FOUND : left > 0 ? RECURSE : NOT_FOUND;
		}
		template <find_if_state>
		struct find_if_impl;
		template <>
		struct find_if_impl<FOUND> {
			// element was found, call the found function with it
			template <template <typename> class Pred, typename Found, typename NotFound,
			          typename Prev, typename... Ts>
			using f = typename Found::template f<Prev>;
		};
		template <>
		struct find_if_impl<NOT_FOUND> {
			// element was not found, call the supplied function
			template <template <typename> class Pred, typename Found, typename NotFound,
			          typename Prev, typename... Ts>
			using f = NotFound;
		};
		template <>
		struct find_if_impl<RECURSE> {
			template <template <typename> class Pred, typename Found, typename NotFound,
			          typename Prev, typename T, typename... Ts>
			using f = typename find_if_impl<find_if_selector(
			        Pred<T>::value, sizeof...(Ts))>::template f<Pred, Found, NotFound, T, Ts...>;
		};
		template <typename L, template <typename> class Pred, typename Found, typename NotFound>
		struct find_if_unpack;
		template <template <typename...> class Seq, typename... Ts, template <typename> class Pred,
		          typename Found, typename NotFound>
		struct find_if_unpack<Seq<Ts...>, Pred, Found, NotFound> {
			using f = typename find_if_impl<find_if_selector(
			        false, sizeof...(Ts))>::template f<Pred, Found, NotFound, void, Ts...>;
		};

		/// find an element that returns true for predicate Pred, calling Found (by calling the
		/// ::f member with the found element) when the element is found, or returning NotFound
		/// when the element is not found
		template <typename L, template <typename> class Pred, typename Found, typename NotFound>
		using find_if = typename find_if_unpack<L, Pred, Found, NotFound>::f;

		template<unsigned N, typename ...Ts>
		struct drop_impl;
		template <unsigned N, typename T, typename... Ts>
		struct drop_impl<N, T, Ts...> {
			using f = typename drop_impl<N - 1, Ts...>::f;
		};
		template<typename T, typename... Ts>
		struct drop_impl<0, T, Ts...> {
			using f = mpl::list<Ts...>;
		};
		template <typename... Ts>
		struct drop_impl<0, Ts...> {
			using f = mpl::list<Ts...>;
		};

		template <unsigned N, typename... Ts>
		using drop = typename drop_impl<N, Ts...>::f;

		template <unsigned N, typename... Ts>
		struct take_impl;
		template <unsigned N, typename T, typename... Ts>
		struct take_impl<N, T, Ts...> {
			template <typename Result>
			using f = push_front<T, typename take_impl<N - 1, Ts...>::template f<Result>>;
		};
		// extra case to solve the ambiguous overload
		template <typename T, typename... Ts>
		struct take_impl<0, T, Ts...> {
			template <typename Result>
			using f = Result;
		};
		template<typename... Ts>
		struct take_impl<0, Ts...> {
			template<typename Result> using f = Result;
		};

		template <unsigned N, typename... Ts>
		using take = typename take_impl<N, Ts...>::template f<mpl::list<>>;
	}
}
