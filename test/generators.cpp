//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <metacheck/metacheck.hpp>
#include <metacheck/generators.hpp>

namespace kmpl = kvasir::mpl;

template <typename Gen>
struct generator_value {
	using type = Gen;
};

template <template <typename...> class Func>
struct create_generator_value {
	template <typename... Ts>
	using f = generator_value<Func<Ts...>>;
};

struct any_generator {
	template <typename seed>
	using generate = typename mc::gen::any<
	        mc::gen::just<mc::gen::uint_<>>, mc::gen::just<mc::gen::bool_>,
	        mc::gen::just<mc::gen::anything>,
	        mc::gen::call<create_generator_value<mc::gen::list>::template f, any_generator>,
	        mc::gen::call_variadic<create_generator_value<mc::gen::list>::template f, any_generator,
	                               mc::gen::uint_<4>>,
	        mc::gen::call<create_generator_value<mc::gen::just>::template f,
	                      mc::gen::anything>>::template generate<seed>;
};

template <typename Gen>
using generator_has_generate          = kmpl::call<kmpl::always<kmpl::bool_<true>>,
                                          typename Gen::template generate<mc::random_seed>>;
const auto generate_has_generate_test = mc::test<generator_has_generate, 100, any_generator>;

const auto generators_section_obj =
        mc::evaluate(mc::section("generators", generate_has_generate_test));
const mc::result *generators_section = &generators_section_obj;
