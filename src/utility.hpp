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
			name.erase(0, 14); // remove "mc::func_name<"
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
			constexpr static bool value = kvasir::mpl::call<kvasir::mpl::all<kvasir::mpl::invert<>>, Ts...>::value;
		};
	}
}
