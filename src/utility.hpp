//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <type_traits>
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
		template <unsigned N>
		struct make_uint_sequence_impl {
			template <typename C, typename... Ts>
			using f = typename make_uint_sequence_impl<N - 1>::template f<
			        C, kvasir::mpl::uint_<N - 1>, Ts...>;
		};
		template <>
		struct make_uint_sequence_impl<0> {
			template <typename C, typename... Ts>
			using f = kvasir::mpl::call<C, Ts...>;
		};

		template <typename C, typename... Ts>
		using uint_sequence_for = typename make_uint_sequence_impl<sizeof...(Ts)>::template f<C>;

		template <unsigned N, typename C>
		using uint_sequence = typename make_uint_sequence_impl<N>::template f<C>;

		template <unsigned N, typename T, typename C>
		using repeat = uint_sequence<N, kvasir::mpl::transform<kvasir::mpl::always<T>, C>>;

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

		/// both values must be equal, otherwise the types are returned
		template <typename A, typename B>
		struct equal {
			constexpr static bool value = A::value == B::value;
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
				return /*n >= 16 ? 16 : n >= 8 ? 8 : n >= 4 ? 4 : n >= 2 ? 2 : */ n >= 1 ? 1 : 0;
			}

			template <unsigned>
			struct fold_transform_impl;

			template <>
			struct fold_transform_impl<0> {
				template <template <typename...> class F, typename State, typename... Ts>
				struct f {
					using type  = kvasir::mpl::detail::rlist_tail_of8;
					using state = State;
				};
			};

			template <>
			struct fold_transform_impl<1> {
				template <template <typename...> class F, typename State, typename T0,
				          typename... Ts>
				struct f {
					using r0  = F<State, T0>;
					using res = typename fold_transform_impl<fold_transform_select(
					        sizeof...(Ts))>::template f<F, typename r0::state, Ts...>;

					using state = typename res::state;
					using type  = kvasir::mpl::detail::rlist<kvasir::mpl::list<typename r0::type>,
					                                        typename res::type>;
				};
			};

			template <>
			struct fold_transform_impl<2> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename... Ts>
				struct f {
					using r0  = F<State, T0>;
					using r1  = F<typename r0::state, T1>;
					using res = typename fold_transform_impl<fold_transform_select(
					        sizeof...(Ts))>::template f<F, typename r1::state, Ts...>;

					using state = typename res::state;
					using type  = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<typename r0::type, typename r1::type>,
					        typename res::type>;
				};
			};

			template <>
			struct fold_transform_impl<4> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename T2, typename T3, typename... Ts>
				struct f {
					using r0  = F<State, T0>;
					using r1  = F<typename r0::state, T1>;
					using r2  = F<typename r1::state, T2>;
					using r3  = F<typename r2::state, T3>;
					using res = typename fold_transform_impl<fold_transform_select(
					        sizeof...(Ts))>::template f<F, typename r3::state, Ts...>;

					using state = typename res::state;
					using type  = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<typename r0::type, typename r1::type,
					                          typename r2::type, typename r3::type>,
					        typename res::type>;
				};
			};

			template <>
			struct fold_transform_impl<8> {
				template <template <typename...> class F, typename State, typename T0, typename T1,
				          typename T2, typename T3, typename T4, typename T5, typename T6,
				          typename T7, typename... Ts>
				struct f {
					using r0  = F<State, T0>;
					using r1  = F<typename r0::state, T1>;
					using r2  = F<typename r1::state, T2>;
					using r3  = F<typename r2::state, T3>;
					using r4  = F<typename r3::state, T4>;
					using r5  = F<typename r4::state, T5>;
					using r6  = F<typename r5::state, T6>;
					using r7  = F<typename r6::state, T7>;
					using res = typename fold_transform_impl<fold_transform_select(
					        sizeof...(Ts))>::template f<F, typename r7::state, Ts...>;

					using state = typename res::state;
					using type  = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<typename r0::type, typename r1::type,
					                          typename r2::type, typename r3::type,
					                          typename r4::type, typename r5::type,
					                          typename r6::type, typename r7::type>,
					        typename res::type>;
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
					using r1  = F<typename r0::state, T1>;
					using r2  = F<typename r1::state, T2>;
					using r3  = F<typename r2::state, T3>;
					using r4  = F<typename r3::state, T4>;
					using r5  = F<typename r4::state, T5>;
					using r6  = F<typename r5::state, T6>;
					using r7  = F<typename r6::state, T7>;
					using r8  = F<typename r7::state, T8>;
					using r9  = F<typename r8::state, T9>;
					using r10 = F<typename r9::state, T10>;
					using r11 = F<typename r10::state, T11>;
					using r12 = F<typename r11::state, T12>;
					using r13 = F<typename r12::state, T13>;
					using r14 = F<typename r13::state, T14>;
					using r15 = F<typename r14::state, T15>;
					using res = typename fold_transform_impl<fold_transform_select(
					        sizeof...(Ts))>::template f<F, typename r15::state, Ts...>;

					using state = typename res::state;
					using type  = kvasir::mpl::detail::rlist<
					        kvasir::mpl::list<
					                typename r0::type, typename r1::type, typename r2::type,
					                typename r3::type, typename r4::type, typename r5::type,
					                typename r6::type, typename r7::type, typename r8::type,
					                typename r9::type, typename r10::type, typename r11::type,
					                typename r12::type, typename r13::type, typename r14::type,
					                typename r15::type>,
					        typename res::type>;
				};
			};
		}

		template <typename State, typename F, typename TC, typename C>
		struct fold_transform {
			template <typename... Ts>
			struct impl {
				using res = typename detail::fold_transform_impl<detail::fold_transform_select(
				        sizeof...(Ts))>::template f<F::template f, State, Ts...>;

				using f =
				        kvasir::mpl::call<C, typename res::state,
				                          kvasir::mpl::call<kvasir::mpl::detail::recursive_join<TC>,
				                                            typename res::type>>;
			};

			template <typename... Ts>
			using f = typename impl<Ts...>::f;
		};
	}
}
