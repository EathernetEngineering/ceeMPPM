/*
 * ceeMPPM
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

#include <i2c_mock.h>
#include <log.h>

#include <chrono>
#include <cmath>
#include <format>
#include <stdexcept>

namespace cee {
namespace platform {
	static auto g_TimeStart = std::chrono::steady_clock::now();
	std::chrono::duration<float, std::milli> ts;

	static uint8_t NextAdcChan0(float f) {
		using namespace std::chrono;
		ts = duration_cast<milliseconds>(steady_clock::now() - g_TimeStart);
		float t = ts.count() / 1000.f;

		float v = 125.f * std::pow(std::sin(t * f), 50.f) +
			25.f * std::pow(std::sin(t * f - 1), 50.f) +
			15.f * std::pow(std::sin(t * f + 1), 50.f) -
			40.f * std::pow(std::sin(t * f - 0.2), 50.f) -
			15.f * std::pow(std::sin(t * f + 0.4), 50.f) +
			20.f;
		return static_cast<uint8_t>(v);
	}

	MockI2CController::MockI2CController(const std::string &file, I2CContextType ctxType, Logger logger)
	 : I2CController(ctxType, logger), m_PrevAddress(0) {
		(void)file;
	}

	MockI2CController::~MockI2CController() {
	}

	void MockI2CController::SelectDevice(uint8_t address) {
		if (address == 0x48) {
			m_PrevAddress = address;
			return;
		}
		m_PrevAddress = 0;
#ifndef NDEBUG
		throw core::InvalidParameter(fmt::format("I2CController::SelectDevice(): Unexpected address {:02X}",
					address));
#endif
	}

	ssize_t MockI2CController::Read(void *data, ssize_t count) {
		if (m_PrevAddress == 0x48 && count == 1) {
			*reinterpret_cast<uint8_t*>(data) = m_PrevAdcVal;
			m_PrevAdcVal = NextAdcChan0(2.5f);
			return count;
		}
		return 0;
	}

	ssize_t MockI2CController::Write(const void *data, ssize_t count) {
		return count;
	}
}
}

