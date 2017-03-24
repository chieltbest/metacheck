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
				template <template <typename...> class ResultList, typename... Ts>
				struct any_shrinker {
					template <typename Idx>
					struct replacer {
						template <typename Shrink>
						struct replace_zip {
							template <typename Elem, typename CurIdx>
							using f = typename mpl::conditional<Idx{} ==
							                                    CurIdx{}>::template f<Shrink, Elem>;
						};
						template <typename Shrink>
						using f = mpl::zip_with<replace_zip<Shrink>::template f, ResultList<Ts...>,
						                        mpl::uint_sequence_for<Ts...>>;
					};
					template <typename Shrinks, typename Idx>
					using zip_func = mpl::transform<replacer<Idx>::template f, Shrinks>;
				};

				template <template <typename...> class ResultList, typename... Ts>
				using shrink_any =
				        mpl::join<mpl::zip_with<any_shrinker<ResultList, Ts...>::template zip_func,
				                                mpl::list<typename Ts::shrink...>,
				                                mpl::uint_sequence_for<Ts...>>>;
			}

			template <typename... Ts>
			struct list {
				using type   = mpl::list<typename Ts::type...>;
				using shrink = detail::shrink_any<value::list, Ts...>;
			};

			template <typename... Ts>
			struct list_of {
				template <typename N>
				using erase = mpl::join<
				        mpl::list<mpl::take<N::value, Ts...>, mpl::drop<N::value + 1, Ts...>>>;

				using remove_any = mpl::join<mpl::transform<erase, mpl::uint_sequence_for<Ts...>>>;

				using type   = mpl::list<typename Ts::type...>;
				using shrink = mpl::join<mpl::list<mpl::list<list_of<>>, remove_any,
				                                   detail::shrink_any<value::list_of, Ts...>>>;
			};
			template <>
			struct list_of<> {
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
		template <unsigned max = 512, unsigned min = 0>
		struct uint_ {
			constexpr static unsigned max_num = max > min ? max : min;
			constexpr static unsigned min_num = min < max ? min : max;

			// generate a new int from a seed
			template <typename seed>
			using generate = detail::gen_result<
			        typename seed::next::next,
			        value::uint_<(seed{} % 3 == 0) ?
			                             min_num : // try the minimum value many times as it has a
			                             // higher chance to fail
			                             ((typename seed::next{} % (max_num - (min_num + 1))) +
			                              (min_num + 1))>>;
		};

		template <typename... Ts>
		struct any {
			template <typename seed>
			using generate = typename mpl::at<(seed{} % sizeof...(Ts)),
			                                  Ts...>::template generate<typename seed::next>;
		};

		namespace detail {
			template <typename... Ts>
			struct generate_all {
				template <typename State, typename Result>
				using gen_func_impl = detail::gen_result<
				        typename Result::next_seed,
				        mpl::push_front<typename Result::type, typename State::type>>;
				template <typename State, typename T>
				using gen_func =
				        gen_func_impl<State,
				                      typename T::template generate<typename State::next_seed>>;

				template <typename seed, typename ResultList>
				using generate =
				        mpl::fold_right<gen_func, detail::gen_result<seed, ResultList>, Ts...>;
			};
		}

		template <typename... Ts>
		struct list {
			template <typename seed>
			using generate =
			        typename detail::generate_all<Ts...>::template generate<seed, value::list<>>;
		};

		template <typename Gen, typename N = uint_<>>
		struct list_of {
			template <typename seed>
			using generate = typename mpl::call<
			        detail::generate_all,
			        mpl::repeat<N::template generate<seed>::type::type::value, Gen>>::
			        template generate<typename N::template generate<seed>::next_seed,
			                          value::list<>>;
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
			        just<void>, just<decltype(nullptr)>, uint_<>,
			        just<std::integral_constant<decltype(nullptr), nullptr>>,
			        just<detail::func_wrap_t<detail::foo_func>>,
			        just<detail::func_wrap_t_ptr_t<nullptr>>, just<detail::inconstructible>,
			        // anything can also be a list of anything, or a list of a list of anything
			        list_of<anything, uint_<5>>>::template generate<seed>;
		};
	}
};
