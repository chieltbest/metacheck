//          Copyright Chiel Douwes 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <kvasir/mpl/mpl.hpp>

#include "generators.hpp"

namespace mc {
	namespace detail {
		namespace kmpl = kvasir::mpl;

		template <typename Case, unsigned shrinks>
		struct minify_result {
			using testcase                          = Case;
			constexpr static unsigned total_shrinks = shrinks;
		};

		template <unsigned shrinks = 0>
		struct pass_minify {
			struct is_empty {
				template <typename... Ts>
				using f = kmpl::bool_<sizeof...(Ts) == 0>;
			};

			template <typename Case>
			using f = kmpl::call<kmpl::unpack<kmpl::if_<is_empty, kmpl::always<minify_result<Case, shrinks>>,
			                                            kmpl::front<pass_minify>>>,
			                     typename mc::gen::shrink<Case>::type>;
		};

		template <unsigned shrinks = 0>
		struct minify {
			template <typename Case>
			using f = typename kmpl::call<
			        kmpl::unpack<kmpl::find_if<kmpl::logical_not<>, kmpl::front<minify<shrinks + 1>>,
			                                   kmpl::always<minify_result<Case, shrinks>>>>,
			        typename mc::gen::shrink<Case>::type>;
		};

		/// get the minified version of the first failed test in the list, return the first test but minified when
		/// all test succeed
		template <typename... Tests>
		using first_minified = kmpl::call<kmpl::find_if<kmpl::logical_not<>, kmpl::front<minify<>>,
		                                                kmpl::always<kmpl::call<kmpl::front<pass_minify<>>, Tests...>>>,
		                                  Tests...>;

	} // namespace detail
} // namespace mc
