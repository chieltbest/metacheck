//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <limits>

#include "random.hpp"

namespace mc {
	namespace gen {
		namespace detail {
			template <typename T>
			struct uint_minify {
				using f = mpl::uint_<(T{} - 1)>;
			};
			template <>
			struct uint_minify<mpl::uint_<0>> {
				// can't minify zero
				using f = mpl::uint_<0>;
			};
		}

		/// uint template object, creates a uint somewhere within the range min to max, inclusive
		template <unsigned max = std::numeric_limits<unsigned>::max(), unsigned min = 0>
		struct uint_ {
			constexpr static unsigned max_num = max > min ? max : min;
			constexpr static unsigned min_num = min < max ? min : max;

			// generate a new int from a seed
			template <seed_t seed>
			using generate = mpl::uint_<((seed % (max_num - min_num)) + min_num)>;

			// try to minify the int type; fails if the int is already 0
			//
			template <typename Old, unsigned type>
			using minify = typename detail::uint_minify<Old>;
		};
	}
};
