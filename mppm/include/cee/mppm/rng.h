/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CEE_RNG_H_
#define CEE_RNG_H_

#include <cee/mppm/log.h>

#include <cstdint>
#include <optional>
#include <random>
#include <type_traits>


namespace cee {
namespace {
	struct rngImpl {
		inline static void Init(std::optional<uint64_t> seed = {}) {
			std::random_device r;
			s_E = std::mt19937_64(seed.value_or(r()));

			if (s_Initialized) {
				CEE_CORE_DEBUG("RNG already initialized. Calling this function again will cause a seed reset.");
				CEE_CORE_DEBUG("\tCalling this function again will cause a seed reset.");
				CEE_CORE_DEBUG("\tTo avoid this message use cee::rng<T>::ResetSeed().");
			}

			s_Initialized = true;
		}

		inline static int64_t next() { return s_E(); }

		inline static void discard(uint64_t z) { s_E.discard(z); }

		inline static void Reset(std::optional<uint64_t> seed = {}) {
			std::random_device r;
			s_E = std::mt19937_64(seed.value_or(r()));
		}

		inline uint64_t operator()() { return s_E(); }

		inline static std::mt19937_64& gen() { return s_E; }

	private:
		inline static bool s_Initialized;
		inline static std::mt19937_64 s_E;
	};
}

template<typename T>
struct rng {
	static_assert(std::is_arithmetic_v<T>, "RNG Type muse be arithmetic type.");
	static void Init(T seed = 0) { rngImpl::Init(seed); }

	static T next() { return rngImpl::next(); }
	T operator()() { next(); }
	static void discard(uint64_t z) { rngImpl::discard(z); }
};

template<>
inline int64_t rng<int64_t>::next() {
	static std::uniform_int_distribution<int64_t> distrib(std::numeric_limits<int64_t>::min(),
													   std::numeric_limits<int64_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline int32_t rng<int32_t>::next() {
	static std::uniform_int_distribution<int32_t> distrib(std::numeric_limits<int32_t>::min(),
													   std::numeric_limits<int32_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline int16_t rng<int16_t>::next() {
	static std::uniform_int_distribution<int16_t> distrib(std::numeric_limits<int16_t>::min(),
													   std::numeric_limits<int16_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline int8_t rng<int8_t>::next() {
	static std::uniform_int_distribution<int8_t> distrib(std::numeric_limits<int8_t>::min(),
													   std::numeric_limits<int8_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline uint64_t rng<uint64_t>::next() {
	static std::uniform_int_distribution<uint64_t> distrib(std::numeric_limits<uint64_t>::min(),
													   std::numeric_limits<uint64_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline uint32_t rng<uint32_t>::next() {
	static std::uniform_int_distribution<uint32_t> distrib(std::numeric_limits<uint32_t>::min(),
													   std::numeric_limits<uint32_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline uint16_t rng<uint16_t>::next() {
	static std::uniform_int_distribution<uint16_t> distrib(std::numeric_limits<uint16_t>::min(),
													   std::numeric_limits<uint16_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline uint8_t rng<uint8_t>::next() {
	static std::uniform_int_distribution<uint8_t> distrib(std::numeric_limits<uint8_t>::min(),
													   std::numeric_limits<uint8_t>::max());
	return distrib(rngImpl::gen());
}

template<>
inline float rng<float>::next() {
	static std::uniform_real_distribution<float> distrib(0.f, 1.f);
	return distrib(rngImpl::gen());
}

template<>
inline double rng<double>::next() {
	static std::uniform_real_distribution<double> distrib(0.0, 1.0);
	return distrib(rngImpl::gen());
}
}

#endif

