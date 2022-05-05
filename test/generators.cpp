//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <metacheck/generators.hpp>
#include <metacheck/metacheck.hpp>

constexpr int num_trials = 100;

namespace kmpl = kvasir::mpl;

// seeds -----------------------------

struct random_seed_gen {
	template <typename T>
	struct value_type {
		using type = T;
	};

	template <typename seed>
	struct generate {
		using next_seed = typename seed::next;

		using type = value_type<seed>;
	};
};

template <typename seed1, typename seed2>
using seeds_dont_repeat           = kmpl::bool_<!std::is_same_v<seed1, seed2>>;
const auto seeds_dont_repeat_test = mc::test<seeds_dont_repeat, num_trials, random_seed_gen, random_seed_gen>;

template <typename seed>
concept is_seed_c = requires {
	                    typename seed::next;
	                    { seed::state } -> std::same_as<uint64_t>;
                    };

// properties for all generators -----------------------------

struct any_generator {
	template <typename seed>
	using generate =
	        typename mc::gen::any<mc::gen::just<mc::gen::uint_<>>, mc::gen::just<mc::gen::bool_>,
	                              mc::gen::just<mc::gen::anything>, mc::gen::call<mc::gen::list, any_generator>,
	                              mc::gen::call_variadic<mc::gen::list, any_generator, mc::gen::uint_<4>>,
	                              mc::gen::call<mc::gen::just, mc::gen::anything>>::template generate<seed>;
};

template <typename G>
concept is_generator_c = requires { typename G::template generate<mc::random_seed>; };

template <typename Gen>
using generator_has_generate          = kmpl::bool_<is_generator_c<Gen>>;
const auto generate_has_generate_test = mc::test<generator_has_generate, num_trials, any_generator>;

template <typename Result>
concept is_gen_result_c = requires {
	                          typename Result::next_seed;
	                          is_seed_c<typename Result::next_seed>;
	                          typename Result::type;
                          };

template <is_generator_c Gen, typename seed>
using generated_is_gen_result           = kmpl::bool_<is_gen_result_c<typename Gen::template generate<seed>>>;
const auto generated_is_gen_result_test = mc::test<generated_is_gen_result, num_trials, any_generator, random_seed_gen>;

// any -----------------------------

template <typename Val>
struct is_value_any {
	using type = kmpl::bool_<false>;
};
template <typename T, typename... Ts>
struct is_value_any<mc::gen::value::any<T, Ts...>> {
	using type = kmpl::bool_<true>;
};

template <typename seed>
using shrunk_is_value_any =
        kmpl::call<kmpl::unpack<kmpl::all<kmpl::cfl<is_value_any>>>,
                   typename mc::gen::shrink<typename mc::gen::anything::template generate<seed>::type>::type>;
const auto shrunk_is_value_any_test = mc::test<shrunk_is_value_any, num_trials, random_seed_gen>;

const auto generators_section_obj =
        mc::evaluate(mc::section("generators", seeds_dont_repeat_test, generate_has_generate_test,
                                 generated_is_gen_result_test, shrunk_is_value_any_test));
const mc::result *generators_section = &generators_section_obj;
