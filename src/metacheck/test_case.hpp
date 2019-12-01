//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "interface.hpp"
#include "utility.hpp"

namespace mc {
	namespace detail {

		/// A single test case with supplied parameters
		/// \tparam Test The test case to be executed
		/// \tparam Params The parameters, wrapped by value wrappers used in minification
		template <typename Test, typename Params>
		struct test_case {
			using params                = Params;
			using test                  = Test;
			using type                  = kmpl::call<kmpl::unpack<Test>, typename Params::type>;
			constexpr static bool value = type::value;
		};

		template <typename Test, typename Seed>
		struct gen_test_cases_impl {};

		template <template <typename...> class Test, typename... Params, typename Seed>
		struct gen_test_cases_impl<test<Test, 0, Params...>, Seed> {
			template <typename... Cases>
			using f = gen::detail::gen_result<Seed, kmpl::list<Cases...>>;
		};

		template <template <typename...> class Test, unsigned tries, typename... Params,
		          typename Seed>
		struct gen_test_cases_impl<test<Test, tries, Params...>, Seed> {
			using gen_params = typename gen::list<Params...>::template generate<Seed>;

			template <typename... Cases>
			using f = typename gen_test_cases_impl<test<Test, tries - 1, Params...>,
			                                       typename gen_params::next_seed>::
			        template f<Cases..., test_case<kmpl::cfe<Test>, typename gen_params::type>>;
		};

		template <typename Test, typename Seed>
		using gen_test_cases = typename gen_test_cases_impl<Test, Seed>::template f<>;

	} // namespace detail

	namespace gen {
		namespace kmpl = kvasir::mpl;
		template <typename Test, typename Params>
		struct shrink<mc::detail::test_case<Test, Params>> {
			using type =
			        kmpl::call<kmpl::unpack<kmpl::transform<
			                           kmpl::push_front<Test, kmpl::cfe<mc::detail::test_case>>>>,
			                   typename shrink<Params>::type>;
		};
	} // namespace gen
} // namespace mc
