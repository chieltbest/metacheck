//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <metacheck/metacheck.hpp>
#include <metacheck/utility.hpp>

constexpr int num_trials = 10;

namespace kmpl = kvasir::mpl;

template <typename T>
struct type_name_not_empty {
	using type                  = mc::type_name<T>;
	static constexpr bool value = true; // no tests in constexpr

	bool operator()() {
		std::string name = type{};
		std::cout << name << std::endl;
		return !name.empty(); // all types should have a name
	}
};
const auto type_name_not_empty_test = mc::test<type_name_not_empty, num_trials, mc::gen::anything>;
// cxa_demangle does not handle big typenames well, test some big names by making lists
const auto type_name_not_empty_list_test =
        mc::test<type_name_not_empty, num_trials, mc::gen::list_of<mc::gen::anything>>;

template <typename List>
struct foreach_anything {
	static constexpr bool value = true; // not all types work in constexpr

	bool operator()() {
		mc::foreach<void>(mc::mpl::call<std::tuple, List>{},
		                  [](auto t) { std::cout << std::string(mc::type_name<decltype(t)>{}) << std::endl; });
		return true;
	}
};

template <typename T, typename N>
using repeat_all_t           = mc::mpl::repeat<N::value, T, kmpl::all<kmpl::same_as<T>>>;
const auto repeat_all_t_test = mc::test<repeat_all_t, num_trials, mc::gen::anything, mc::gen::uint_<>>;

const auto utility_section_obj = mc::evaluate(
        mc::section("utility", type_name_not_empty_test, type_name_not_empty_list_test, repeat_all_t_test));
const mc::result *utility_section = &utility_section_obj;
