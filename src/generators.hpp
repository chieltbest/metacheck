//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// just like rapidcheck, metacheck has the concept of "generators" and "values";
// generators are able to generate a value when given a random seed, and values are generated
// objects with the minify member function and a type member class that specifies the generated
// type
#pragma once

#include <limits>

#include "random.hpp"
#include "mpl.hpp"

namespace mc {
	namespace gen {
		namespace value {
			template <typename T>
			struct just {
				using type   = T;
				using shrink = mpl::list<>;
			};

			template <unsigned value>
			struct uint_ {
				using type   = mpl::uint_<value>;
				using shrink = mpl::list<uint_<0>, uint_<value / 2>, uint_<value - 1>>;
			};
			template <>
			struct uint_<0> {
				using type = mpl::uint_<0>;
				// unable to shrink 0
				using shrink = mpl::list<>;
			};

			namespace detail {
				template <typename Ints, typename... Ts>
				struct list_shrink_impl;
			}

			template <typename... Ts>
			struct list {
				template <typename Idx>
				struct replacer {
					template <typename Shrink>
					struct replace_zip {
						template <typename Elem, typename CurIdx>
						using f = typename mpl::conditional<Idx{} == CurIdx{}>::template f<Shrink,
						                                                                   Elem>;
					};
					template <typename Shrink>
					using f = mpl::zip_with<replace_zip<Shrink>::template f, list<Ts...>,
					                        mpl::uint_sequence_for<Ts...>>;
				};
				template <typename Shrinks, typename Idx>
				using zip_func = mpl::transform<replacer<Idx>::template f, Shrinks>;

				using shrink = mpl::join<mpl::zip_with<zip_func, mpl::list<typename Ts::shrink...>,
				                                       mpl::uint_sequence_for<Ts...>>>;

				using type = mpl::list<typename Ts::type...>;
			};
			template <>
			struct list<> {
				using type   = mpl::list<>;
				using shrink = mpl::list<>;
			};
		}

		namespace detail {
			template <typename seed, typename T>
			struct gen_result {
				using next_seed = seed;

				using type = T;
			};
		}

		/// just use the value provided, do not shrink
		template <typename T>
		struct just {
			template <typename seed>
			using generate = detail::gen_result<seed, value::just<T>>;
		};

		/// uint template object, creates a uint somewhere within the range min to max, inclusive
		template <unsigned max = 2048, unsigned min = 0>
		struct uint_ {
			constexpr static unsigned max_num = max > min ? max : min;
			constexpr static unsigned min_num = min < max ? min : max;

			// generate a new int from a seed
			template <typename seed>
			using generate = detail::gen_result<
			        typename seed::next::next,
			        value::uint_<(seed{} % 2 == 0) ?
			                             min_num : // try the minimum value many times as it has a
			                             // higher chance to fail
			                             ((typename seed::next{} % (max_num - min_num)) +
			                              min_num)>>;
		};

		template <typename... Ts>
		struct any {
			template <typename seed>
			using generate = typename mpl::at<(seed{} % sizeof...(Ts)),
			                                  Ts...>::template generate<typename seed::next>;
		};

		template <typename... Ts>
		struct list {
			template <typename State, typename Result>
			using gen_func_impl = detail::gen_result<
			        typename Result::next_seed,
			        mpl::push_front<typename Result::type, typename State::type>>;
			template <typename State, typename T>
			using gen_func =
			        gen_func_impl<State, typename T::template generate<typename State::next_seed>>;

			template <typename seed>
			using generate = mpl::fold<gen_func, detail::gen_result<seed, value::list<>>, Ts...>;
		};
	}
};
