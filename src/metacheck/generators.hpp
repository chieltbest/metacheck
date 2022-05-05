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
			};

			template <unsigned value>
			struct uint_ { // TODO add minimize constraints for minimum value
				using type = mpl::uint_<value>;
			};

			template <bool value>
			struct bool_ {
				using type = mpl::bool_<value>;
			};

			template <typename T, typename... Ts>
			struct any {
				using type = typename T::type;
			};

			template <typename... Ts>
			struct list {
				using type = mpl::list<typename Ts::type...>;
			};

			template <typename... Ts>
			struct list_of { // TODO adhere to int generator size requirements
				using type = mpl::list<typename Ts::type...>;
			};

			template <template <typename...> class F, typename... Ts>
			struct call {
				using type = F<typename Ts::type...>;
			};

			template <template <typename...> class F, typename... Ts>
			struct call_variadic {
				using type = F<typename Ts::type...>;
			};
		} // namespace value

		namespace detail {
			template <typename seed, typename T>
			struct gen_result {
				using next_seed = seed;

				using type = T;
			};
		} // namespace detail

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
			using generate = detail::gen_result<typename seed::next::next,
			                                    value::uint_<(((unsigned(seed{}) >> (typename seed::next{} % 32))
			                                                   // random distribution weighted towards small values
			                                                   % (max_num - (min_num))) +
			                                                  (min_num))>>;
		};

		struct bool_ {
			template <typename seed>
			using generate = detail::gen_result<typename seed::next, value::bool_<((seed{} % 2) == 1)>>;
		};

		namespace detail {
			template <typename Seed, typename Elem>
			using gen_func = typename Elem::template generate<typename Seed::next_seed>;

			struct get_type {
				template <typename T>
				using f = typename T::type;
			};

			struct generate_all_to_gen_result {
				template <typename Last, typename List>
				using f = gen_result<typename Last::next_seed, List>;
			};

			template <typename seed, typename ResultList>
			using generate_all =
			        mc::mpl::fold_transform<gen_result<seed, void>, mpl::cfe<gen_func>,
			                                mpl::transform<get_type, ResultList>, generate_all_to_gen_result>;
		} // namespace detail

		template <typename... Ts>
		struct any {
			template <typename seed>
			struct generate {
				using random_value = typename mpl::call<mpl::at<mpl::uint_<(seed{} % sizeof...(Ts))>>,
				                                        Ts...>::template generate<typename seed::next>;

				// skip one seed as it is used for the alternatives
				using next_seed = typename random_value::next_seed::next;

				// use a single seed for all the alternatives as they are all exclusive anyways
				using type = value::any<typename random_value::type>;
			};
		};

		namespace detail {
			template <typename Func, typename... Ts>
			struct call {
				template <typename seed>
				using generate = mpl::call<typename detail::generate_all<seed, Func>, Ts...>;
			};

			template <typename Func, typename Gen, typename N = uint_<256>>
			struct call_variadic {
				template <typename seed>
				using generate =
				        mc::mpl::repeat<N::template generate<seed>::type::type::value, Gen,
				                        detail::generate_all<typename N::template generate<seed>::next_seed, Func>>;
			};
		} // namespace detail

		template <typename... Ts>
		struct list {
			template <typename seed>
			using generate = typename detail::call<mpl::cfe<value::list>, Ts...>::template generate<seed>;
		};

		template <typename Gen, typename N = uint_<256>>
		struct list_of {
			template <typename seed>
			using generate = typename detail::call_variadic<mpl::cfe<value::list_of>, Gen, N>::template generate<seed>;
		};

		template <template <typename...> class Func, typename... Ts>
		struct call {
			struct call_value_maker {
				template <typename... Ts2>
				using f = value::call<Func, Ts2...>;
			};

			template <typename seed>
			using generate = mpl::call<typename detail::generate_all<seed, call_value_maker>, Ts...>;
		};

		template <template <typename...> class Func, typename Gen, typename N = uint_<256>>
		struct call_variadic {
			struct call_value_maker {
				template <typename... Ts2>
				using f = value::call_variadic<Func, Ts2...>;
			};

			template <typename seed>
			using generate = mc::mpl::repeat<
			        N::template generate<seed>::type::type::value, Gen,
			        detail::generate_all<typename N::template generate<seed>::next_seed, call_value_maker>>;
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
				// no standard destructor
				~inconstructible()          = delete;

				// no copy constructor
				constexpr inconstructible(inconstructible &)  = delete;
				// no move constructor
				constexpr inconstructible(inconstructible &&) = delete;

				// no copy assignment
				constexpr void operator=(inconstructible &)  = delete;
				// no move assignment
				constexpr void operator=(inconstructible &&) = delete;
			};

			struct bitfield {
				int foo : 1;
			};
			constexpr bitfield bfield_foo = {};

			using bitfield_ref = decltype(bitfield::foo);
		} // namespace detail

		/// can literally be any type, everything is allowed
		struct anything {
			template <typename seed>
			using generate =
			        typename any<uint_<>, just<void>, just<void *>, just<char &&>, just<char *&>, just<void (*)()>,
			                     just<char[]>, just<char (*)[]>, just<char (&)[]>, just<decltype(nullptr)>,
			                     just<std::integral_constant<decltype(nullptr), nullptr>>,
			                     just<detail::inconstructible>, just<detail::func_wrap_t<detail::foo_func>>,
			                     just<detail::func_wrap_t_ptr_t<nullptr>>,

			                     // anything can also be a list of anything, or a list of a list of anything
			                     list_of<anything, uint_<5>>>::template generate<seed>;
		};
	} // namespace gen

	// value shrinking functions
	namespace gen {
		template <typename T>
		struct shrink {
			using type = mpl::list<>;
		};

		template <typename T>
		struct shrink<value::just<T>> {
			using type = mpl::list<>;
		};

		template <unsigned val>
		struct shrink<value::uint_<val>> {
			using type = mpl::list<value::uint_<0>, value::uint_<val / 2>, value::uint_<val - 1>>;
		};

		template <>
		struct shrink<value::uint_<0>> {
			using type = mpl::list<>;
		};

		template <>
		struct shrink<value::bool_<true>> {
			using type = mpl::list<value::bool_<false>>;
		};

		template <>
		struct shrink<value::bool_<false>> {
			using type = mpl::list<>;
		};

		template <typename T, typename... Ts>
		struct shrink<value::any<T, Ts...>> {
			using type = mpl::call<mpl::join<>, mpl::list<value::any<Ts>...>,
			                       mpl::call<mpl::cfl<shrink, mpl::unpack<mpl::transform<mpl::cfe<value::any>>>>, T>>;
		};

		namespace detail {
			template <typename ResultList>
			struct shrink_any {
				template <typename... Ts>
				struct impl {
					template <typename Idx>
					struct replace {
						template <typename Shrink>
						using f = mpl::call<mpl::fork<mpl::take<Idx>, mpl::always<mpl::list<Shrink>>,
						                              mpl::drop<mpl::uint_<Idx::value + 1>>, mpl::join<ResultList>>,
						                    Ts...>;
					};
					template <typename Shrinks, typename Idx>
					using f = mpl::call<mpl::unpack<mpl::transform<replace<Idx>>>, Shrinks>;
				};

				template <typename... Ts>
				using f = mpl::call<mpl::fork<mpl::transform<mpl::cfl<shrink>>, mc::mpl::uint_sequence_for<>,
				                              mpl::zip_with<impl<Ts...>, mpl::join<>>>,
				                    Ts...>;
			};

			template <typename ResultList>
			struct shrink_list_of_impl {
				template <typename N>
				using erase = mpl::fork<mpl::take<N>, mpl::drop<mpl::uint_<N::value + 1>>,
				                        mpl::join<mpl::cfe<value::list_of>>>;

				template <typename... Ts>
				using halve = mpl::call<mpl::fork<mpl::drop<mpl::uint_<(sizeof...(Ts) + 1) / 2>, ResultList>,
				                                  mpl::take<mpl::uint_<sizeof...(Ts) / 2>, ResultList>, mpl::listify>,
				                        Ts...>;

				using impl = mpl::fork<
				        mpl::always<mpl::list<typename ResultList::template f<>>>, mpl::cfe<halve>,
				        mpl::fork_front<mc::mpl::uint_sequence_for<mpl::transform<
				                                mpl::cfe<erase>,
				                                mpl::push_front<mpl::listify, mpl::cfe<mpl::detail::fork_impl>>>>,
				                        mpl::call_f<>>,
				        detail::shrink_any<ResultList>, mpl::join<>>;
			};

			template <typename ResultList>
			using shrink_list_of = typename shrink_list_of_impl<ResultList>::impl;
		} // namespace detail
		template <typename... Ts>
		struct shrink<value::list_of<Ts...>> { // TODO adhere to N generator minify
			using type = mpl::call<detail::shrink_list_of<mpl::cfe<value::list_of>>, Ts...>;
		};
		template <>
		struct shrink<value::list_of<>> {
			using type = mpl::list<>;
		};

		template <typename... Ts>
		struct shrink<value::list<Ts...>> {
			using type = mpl::call<detail::shrink_any<mpl::cfe<value::list>>, Ts...>;
		};

		template <template <typename...> class F, typename... Ts>
		struct shrink<value::call<F, Ts...>> {
			struct call_value_maker {
				template <typename... Ts2>
				using f = value::call<F, Ts2...>;
			};

			using type = mpl::call<detail::shrink_any<call_value_maker>, Ts...>;
		};

		template <template <typename...> class F, typename... Ts>
		struct shrink<value::call_variadic<F, Ts...>> {
			struct call_value_maker {
				template <typename... Ts2>
				using f = value::call<F, Ts2...>;
			};

			using type = mpl::call<detail::shrink_list_of<call_value_maker>, Ts...>;
		};

		template <template <typename...> class F>
		struct shrink<value::call_variadic<F>> {
			using type = mpl::list<>;
		};
	} // namespace gen
}; // namespace mc
