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
#include <type_traits>

#include <kvasir/mpl/mpl.hpp>

#include "random.hpp"
#include "utility.hpp"

namespace mc {
	namespace gen {
		namespace mpl = kvasir::mpl;

		namespace value {
			template <typename T>
			struct just {
				using type = T;
				template <typename>
				using shrink = mpl::list<>;
			};

			template <unsigned value>
			struct uint_ {
				using type = mpl::uint_<value>;
				template <typename>
				using shrink = mpl::list<value::uint_<0>, value::uint_<value / 2>,
				                         value::uint_<value - 1>>;
			};
			template <>
			struct uint_<0> {
				using type = mpl::uint_<0>;
				// unable to shrink 0
				template <typename>
				using shrink = mpl::list<>;
			};

			template <typename T, typename... Ts>
			struct any {
				using type = typename T::type;
				template <typename seed>
				using shrink = mpl::call<mpl::join<>,
				                         mpl::list<typename Ts::template generate<seed>::type...>,
				                         typename T::template shrink<seed>>;
			};

			namespace detail {
				template <typename ResultList, typename... Ts>
				struct any_shrinker {
					template <typename Idx>
					struct replace {
						template <typename Shrink>
						using f = mpl::call<mpl::fork<mpl::join<ResultList>, mpl::take<Idx>,
						                              mpl::always<mpl::list<Shrink>>,
						                              mpl::drop<mpl::uint_<Idx::value + 1>>>,
						                    Ts...>;
					};
					template <typename Shrinks, typename Idx>
					using f = mpl::call<mpl::transform<replace<Idx>>, Shrinks>;
				};

				template <typename seed, typename ResultList, typename... Ts>
				using shrink_any =
				        mpl::call<mpl::zip_with<any_shrinker<ResultList, Ts...>, mpl::join<>>,
				                  mpl::list<typename Ts::template shrink<seed>...>,
				                  mc::mpl::uint_sequence_for<mpl::listify, Ts...>>;
			}

			template <typename... Ts>
			struct list {
				using type = mpl::list<typename Ts::type...>;
				template <typename seed>
				using shrink = detail::shrink_any<seed, mpl::cfe<value::list>, Ts...>;
			};

			template <typename... Ts>
			struct list_of {
				template <typename N>
				using erase = mpl::call<mpl::fork<mpl::join<mpl::cfe<value::list_of>>, mpl::take<N>,
				                                  mpl::drop<mpl::uint_<N::value + 1>>>,
				                        Ts...>;

				using type = mpl::list<typename Ts::type...>;
				template <typename seed>
				using shrink = mpl::call<
				        mpl::join<>, mpl::list<value::list_of<>>,
				        mpl::call<mpl::fork<mpl::listify, mpl::drop<mpl::uint_<sizeof...(Ts) / 2>,
				                                                    mpl::cfe<value::list_of>>,
				                            mpl::take<mpl::uint_<sizeof...(Ts) / 2>,
				                                      mpl::cfe<value::list_of>>>,
				                  Ts...>,
				        mc::mpl::uint_sequence_for<mpl::transform<mpl::cfe<erase>>, Ts...>,
				        detail::shrink_any<seed, mpl::cfe<value::list_of>, Ts...>>;
			};
			template <>
			struct list_of<> {
				using type = mpl::list<>;
				template <typename>
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
			        value::uint_<(seed{} % 8 == 0) ?
			                             min_num : // try the minimum value many times as it has a
			                             // higher chance of failing
			                             ((typename seed::next{} % (max_num - (min_num))) +
			                              (min_num + 1))>>;
		};

		template <typename... Ts>
		struct any {
			template <typename seed>
			struct generate {
				using random_value =
				        typename mpl::call<mpl::at<mpl::uint_<(seed{} % sizeof...(Ts))>>,
				                           Ts...>::template generate<typename seed::next>;

				// skip one seed as it is used for the alternatives
				using next_seed = typename random_value::next_seed::next;

				// use a single seed for all the alternatives as they are all exclusive anyways
				using type = value::any<typename random_value::type, Ts...>;
			};
		};

		namespace detail {
			template <typename State, typename Result>
			using gen_func_impl = detail::gen_result<
			        typename Result::next_seed,
			        mpl::push_front<typename Result::type, typename State::type>>;
			template <typename State, typename T>
			using gen_func =
			        gen_func_impl<State, typename T::template generate<typename State::next_seed>>;

			struct gen_make_result {
				template <typename Result>
				using f = gen_result<typename Result::next_seed, mpl::call<typename Result::type>>;
			};

			template <typename seed, template <typename...> class ResultList>
			using generate_all =
			        mpl::push_front<detail::gen_result<seed, mpl::cfe<ResultList>>,
			                        mpl::fold_right<mpl::cfe<gen_func>, gen_make_result>>;
		}

		template <typename... Ts>
		struct list {
			template <typename seed>
			using generate = mpl::call<typename detail::generate_all<seed, value::list>, Ts...>;
		};

		template <typename Gen, typename N = uint_<256>>
		struct list_of {
			template <typename seed>
			using generate = mc::mpl::repeat<
			        N::template generate<seed>::type::type::value, Gen,
			        detail::generate_all<typename N::template generate<seed>::next_seed,
			                             value::list_of>>;
		};

		namespace detail {
			template <typename...>
			struct foo_func {};
			template <template <typename...> class>
			struct func_wrap_t {};
			template <const func_wrap_t<foo_func> *>
			struct func_wrap_t_ptr_t {};

			struct inconstructible {
				// no standard constructor
				constexpr inconstructible() = delete;
				// no copy constructor
				constexpr inconstructible(inconstructible &) = delete;
				// no move constructor
				constexpr inconstructible(inconstructible &&) = delete;
			};
		}

		/// can literally be any type, everything is allowed
		struct anything {
			template <typename seed>
			using generate = typename any<
			        uint_<>, just<void>, just<void *>, just<char &&>, just<char *&>,
			        just<decltype(nullptr)>,
			        just<std::integral_constant<decltype(nullptr), nullptr>>,
			        just<detail::inconstructible>
			        // just<detail::func_wrap_t<detail::foo_func>>,
			        // just<detail::func_wrap_t_ptr_t<nullptr>>

			        // anything can also be a list of anything, or a list of a list of anything
			        /*list_of<anything, uint_<5>>*/>::template generate<seed>;
		};
	}
};
