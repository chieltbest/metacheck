//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <type_traits>
#include <utility>
#include <kvasir/mpl/mpl.hpp>

#include <string>
#include <typeinfo>

#ifdef __GNUG__
#include <cxxabi.h>

#endif

namespace mc {
#ifdef __GNUG__

	template <typename T>
	struct type_name {
		operator std::string() const {
			const char *name{
			        __cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr)};
			std::string res{name}; // copy from name into string
			delete name;
			return res;
		}
	};

	template <template <typename...> class F>
	struct func_name {
		operator std::string() const {
			std::string name{type_name<func_name<F>>{}};
			std::size_t pos = name.rfind("::");
			if (pos != name.npos) {
				name.erase(0, pos + 2);
			} else {
				name.erase(0, 14); // remove "mc::func_name<"
			}
			name.pop_back(); // remove trailing '>'
			return name;
		}
	};

#else

	template <typename T>
	struct type_name {
		operator std::string() const {
			return {typeid(T).name()};
		}
	};

	template <template <typename...> class F>
	struct func_name {
		operator std::string() const {
			return {typeid(func_name<F>).name()};
		}
	};
#endif

	// define function specification
	template <typename Ostream, typename String, typename... Ts>
	Ostream &&print_all(Ostream &&stream, String &&prepend, Ts &&... ts);

	template <typename Ostream, typename String>
	Ostream &&print_all(Ostream &&stream, String &&prepend) {
		return stream;
	}

	template <typename Ostream, typename String, typename T, typename... Ts>
	auto print_all(Ostream &&stream, String &&prepend, T &&t, Ts &&... ts)
	        -> decltype(print_all(stream << prepend << t, prepend, ts...)) {
		return print_all(stream << prepend << t, prepend, ts...);
	};

	template <typename List>
	struct print_all_list;

	template <template <typename...> class L, typename... Ts>
	struct print_all_list<L<Ts...>> {
		template <typename Ostream>
		constexpr auto operator()(Ostream &&stream) const
		        -> decltype(print_all(stream, "", Ts{}...)) {
			return print_all(stream, "", Ts{}...);
		}
	};

	namespace mpl {
		template <typename T>
		struct make_uint_sequence_impl;

		template <typename C, typename... Ts>
		using uint_sequence_for =
		        kvasir::mpl::call<kvasir::mpl::make_int_sequence<kvasir::mpl::identity, C>,
		                          kvasir::mpl::uint_<sizeof...(Ts)>>;

		template <unsigned N, typename T, typename C>
		using repeat = kvasir::mpl::call<kvasir::mpl::make_int_sequence<kvasir::mpl::always<T>, C>,
		                                 kvasir::mpl::uint_<N>>;

		template <typename...>
		constexpr bool always_false = false;

		/// throw a static assertion that prints the types Ts
		template <typename... Ts>
		struct assert_false {
			static_assert(always_false<Ts...>, "");
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

		// utility functions for convenience within tests

		namespace detail {
			// sfinae test for value
			template <typename A, typename B, bool r = A::value == B::value>
			constexpr bool equal_value(int) {
				return r;
			};

			template <typename, typename>
			constexpr bool equal_value(bool) {
				return false;
			}
		}

		/// both values must be equal, otherwise the types are returned
		template <typename A, typename B>
		struct equal {
			constexpr static bool value = detail::equal_value<A, B>(0);
		};

		template <typename A>
		struct equal<A, A> {
			constexpr static bool value = true;
		};

		/// function where all parameters must be true, if not all the types will be returned
		template <typename... Ts>
		struct all {
			constexpr static bool value =
			        kvasir::mpl::call<kvasir::mpl::all<kvasir::mpl::identity>, Ts...>::value;
		};

		template <typename... Ts>
		struct none {
			constexpr static bool value =
			        kvasir::mpl::call<kvasir::mpl::all<kvasir::mpl::invert<>>, Ts...>::value;
		};

		namespace detail {
			constexpr unsigned fold_transform_select(unsigned n) {
				return n >= 32 ?
				               32 :
				               n >= 16 ? 16 : n >= 8 ? 8 : n >= 4 ? 4 : n >= 2 ? 2 : n >= 1 ? 1 : 0;
			}

			template <unsigned>
			struct fold_transform_impl;

			template <>
			struct fold_transform_impl<0> {
				template <template <typename...> class F, typename State, typename... Ts>
				struct f {
					using type = kvasir::mpl::detail::rlist_tail_of8;
				};
			};

			template <>
			struct fold_transform_impl<1> {
				template <template <typename...> class F, typename State, typename T0,
				          typename... Ts>
				struct f {
					using r0 = F<State, T0>;

					using type = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<r0>,
					        typename fold_transform_impl<fold_transform_select(
					                sizeof...(Ts))>::template f<F, r0, Ts...>::type>;
				};
			};

			template <>
			struct fold_transform_impl<2> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename... Ts>
				struct f {
					using r0 = F<State, T0>;
					using r1 = F<r0, T1>;

					using type = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<r0, r1>,
					        typename fold_transform_impl<fold_transform_select(
					                sizeof...(Ts))>::template f<F, r1, Ts...>::type>;
				};
			};

			template <>
			struct fold_transform_impl<4> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename T2, typename T3, typename... Ts>
				struct f {
					using r0 = F<State, T0>;
					using r1 = F<r0, T1>;
					using r2 = F<r1, T2>;
					using r3 = F<r2, T3>;

					using type = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<r0, r1, r2, r3>,
					        typename fold_transform_impl<fold_transform_select(
					                sizeof...(Ts))>::template f<F, r3, Ts...>::type>;
				};
			};

			template <>
			struct fold_transform_impl<8> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename T2, typename T3, typename T4, typename T5, typename T6,
				          typename T7, typename... Ts>
				struct f {
					using r0 = F<State, T0>;
					using r1 = F<r0, T1>;
					using r2 = F<r1, T2>;
					using r3 = F<r2, T3>;
					using r4 = F<r3, T4>;
					using r5 = F<r4, T5>;
					using r6 = F<r5, T6>;
					using r7 = F<r6, T7>;

					using type = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<r0, r1, r2, r3, r4, r5, r6, r7>,
					        typename fold_transform_impl<fold_transform_select(
					                sizeof...(Ts))>::template f<F, r7, Ts...>::type>;
				};
			};

			template <>
			struct fold_transform_impl<16> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename T2, typename T3, typename T4, typename T5, typename T6,
				          typename T7, typename T8, typename T9, typename T10, typename T11,
				          typename T12, typename T13, typename T14, typename T15, typename... Ts>
				struct f {
					using r0  = F<State, T0>;
					using r1  = F<r0, T1>;
					using r2  = F<r1, T2>;
					using r3  = F<r2, T3>;
					using r4  = F<r3, T4>;
					using r5  = F<r4, T5>;
					using r6  = F<r5, T6>;
					using r7  = F<r6, T7>;
					using r8  = F<r7, T8>;
					using r9  = F<r8, T9>;
					using r10 = F<r9, T10>;
					using r11 = F<r10, T11>;
					using r12 = F<r11, T12>;
					using r13 = F<r12, T13>;
					using r14 = F<r13, T14>;
					using r15 = F<r14, T15>;

					using type = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
					                          r13, r14, r15>,
					        typename fold_transform_impl<fold_transform_select(
					                sizeof...(Ts))>::template f<F, r15, Ts...>::type>;
				};
			};

			template <>
			struct fold_transform_impl<32> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename T2, typename T3, typename T4, typename T5, typename T6,
				          typename T7, typename T8, typename T9, typename T10, typename T11,
				          typename T12, typename T13, typename T14, typename T15, typename T16,
				          typename T17, typename T18, typename T19, typename T20, typename T21,
				          typename T22, typename T23, typename T24, typename T25, typename T26,
				          typename T27, typename T28, typename T29, typename T30, typename T31,
				          typename... Ts>
				struct f {
					using r0  = F<State, T0>;
					using r1  = F<r0, T1>;
					using r2  = F<r1, T2>;
					using r3  = F<r2, T3>;
					using r4  = F<r3, T4>;
					using r5  = F<r4, T5>;
					using r6  = F<r5, T6>;
					using r7  = F<r6, T7>;
					using r8  = F<r7, T8>;
					using r9  = F<r8, T9>;
					using r10 = F<r9, T10>;
					using r11 = F<r10, T11>;
					using r12 = F<r11, T12>;
					using r13 = F<r12, T13>;
					using r14 = F<r13, T14>;
					using r15 = F<r14, T15>;
					using r16 = F<r15, T16>;
					using r17 = F<r16, T17>;
					using r18 = F<r17, T18>;
					using r19 = F<r18, T19>;
					using r20 = F<r19, T20>;
					using r21 = F<r20, T21>;
					using r22 = F<r21, T22>;
					using r23 = F<r22, T23>;
					using r24 = F<r23, T24>;
					using r25 = F<r24, T25>;
					using r26 = F<r25, T26>;
					using r27 = F<r26, T27>;
					using r28 = F<r27, T28>;
					using r29 = F<r28, T29>;
					using r30 = F<r29, T30>;
					using r31 = F<r30, T31>;

					using type = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
					                          r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23,
					                          r24, r25, r26, r27, r28, r29, r30, r31>,
					        typename fold_transform_impl<fold_transform_select(
					                sizeof...(Ts))>::template f<F, r31, Ts...>::type>;
				};
			};
		}

		template <typename State, typename F, typename C>
		struct fold_transform {
			template <typename... Ts>
			using f = kvasir::mpl::call<
			        kvasir::mpl::detail::recursive_join<C>,
			        typename detail::fold_transform_impl<detail::fold_transform_select(
			                sizeof...(Ts))>::template f<F::template f, State, Ts...>::type>;
		};
	}

	/// standard function properties that can be tested against
	namespace prop {
		/// split and combined list calls give equal results
		template <template <typename...> class F, template <typename...> class Join,
		          template <typename...> class Comb, typename L1, typename L2>
		using distributive = mpl::equal<F<Join<L1, L2>>, Comb<F<L1>, F<L2>>>;

		/// reversed arguments gives equal results
		template <template <typename...> class F, typename L1, typename L2>
		using commutative = mpl::equal<F<L1, L2>, F<L2, L1>>;

		/// reversed function call sequence gives equal results
		template <template <typename...> class F1, template <typename...> class F2, typename L>
		using associative = mpl::equal<F1<F2<L>>, F2<F1<L>>>;
	}
}
