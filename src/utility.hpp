//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

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
	template<typename Ostream, typename String, typename ...Ts>
	Ostream &&print_all(Ostream &&stream, String &&prepend, Ts&&... ts);

	template <typename Ostream, typename String>
	Ostream &&print_all(Ostream &&stream, String &&prepend) {
		return stream;
	}

	template <typename Ostream, typename String, typename T, typename... Ts>
	auto print_all(Ostream &&stream, String &&prepend, T &&t, Ts &&... ts) -> decltype(print_all(
		stream << prepend << t, prepend, ts...)) {
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
}
