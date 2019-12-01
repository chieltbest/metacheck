//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <kvasir/mpl/mpl.hpp>

namespace mc {
	template <uint64_t cur_state>
	struct seed_state {
		constexpr static uint64_t state = cur_state;

		/// get the next seed from the prng based on the seed that was previously generated
		/// this implementation is the "xorshift*" number generator, chosen for its word size and
		/// good hashing of values
		/// to be noted is that this version is made to be compatible with c++11 constexpr, which
		/// is why it looks so convoluted
		using next = seed_state<(state ^ (state >> 12)) ^ ((state ^ (state >> 12)) << 25) ^
		                        ((state ^ (state >> 12)) ^ ((state ^ (state >> 12)) << 25) >> 27)>;

		constexpr operator uint64_t() const {
			return state * uint64_t(0x2545F4914F6CDD1D);
		}
	};

#ifdef METACHECK_RANDOM
	using random_seed = seed_state<uint64_t(METACHECK_RANDOM)>;
#else
	// initialize the seed with the hashed time for randomness sake
	using random_seed =
	        seed_state<seed_state<(((((__TIME__[1] - '0') + (__TIME__[0] - '0') * 10) * 60) +
	                                ((__TIME__[4] - '0') + (__TIME__[3] - '0') * 10)) *
	                               60) +
	                              ((__TIME__[7] - '0') + (__TIME__[6] - '0') * 10)>{}>;
#endif

	namespace detail {
		template <typename Seed, uint64_t i>
		using encode_int
		        // xor the int with the output of the current seed state, then get the next hash to
		        // distribute the random bits
		        = typename seed_state<Seed{} ^ i>::next;

		constexpr uint64_t encode_string(uint64_t seed, const char *s, unsigned pos) {
			return s[pos] == '\0' ? seed
			                        // barrel shift left by 8 then xor with current char
			                        :
			                        encode_string((((seed & ~(1ull << 56)) << 8) |
			                                       ((seed & (0xffull << 56)) >> 56)) ^
			                                              s[pos],
			                                      s, pos + 1);
		}
	} // namespace detail

#define FILE_RANDOM         \
	mc::detail::encode_int< \
	        mc::seed_state<mc::detail::encode_string(mc::random_seed{}, __FILE__, 0)>, __LINE__>
} // namespace mc
