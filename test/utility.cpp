//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <metacheck/metacheck.hpp>
#include <metacheck/utility.hpp>

namespace kmpl = kvasir::mpl;

template <typename T, typename N>
using repeat_all_t           = mc::mpl::repeat<N::value, T, kmpl::all<kmpl::same_as<T>>>;
const auto repeat_all_t_test = mc::test<repeat_all_t, 100, mc::gen::anything, mc::gen::uint_<>>;

template <typename T>
struct type_name_not_empty {
	using type                  = mc::type_name<T>;
	static constexpr bool value = true; // no tests in constexpr

	bool operator()() {
		return !std::string(type{}).empty(); // all types should have a name
	}
};
const auto type_name_not_empty_test = mc::test<type_name_not_empty, 100, mc::gen::anything>;
const auto type_name_not_empty_list_test =
        mc::test<type_name_not_empty, 100, mc::gen::list_of<mc::gen::anything>>;

const auto utility_section_obj    = mc::evaluate(mc::section(
        "utility", repeat_all_t_test, type_name_not_empty_test, type_name_not_empty_list_test));
const mc::result *utility_section = &utility_section_obj;
