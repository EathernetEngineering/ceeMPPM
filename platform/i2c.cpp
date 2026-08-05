/*
 * ceeMPPM
 * Copyright (C) 2026 Chloe Eather
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

#include <cee/platform/i2c.h>
#include <config.h>
#include <log.h>

#include <i2c_mock.h>
#include <i2c_hw.h>

namespace cee {
namespace platform {
	std::shared_ptr<I2CController> I2CController::Create(const std::string &file,
			I2CContextType ctxType,
			Logger logger) {
		switch (ctxType) {
			case I2CContextType::PLATFORM_I2C_CONTEXT_HW:
#if defined(BUILD_PLATFORM_I2C_HW) && BUILD_PLATFORM_I2C_HW
				return std::shared_ptr<I2CController>(new HwI2CController(file, ctxType, logger));
#else
				error(logger, "Cannot use I2C hardware context. Not built in this version");
				return nullptr;
#endif
			case I2CContextType::PLATFORM_I2C_CONTEXT_MOCK:
#if defined(BUILD_PLATFORM_I2C_MOCK) && BUILD_PLATFORM_I2C_MOCK
				return std::shared_ptr<I2CController>(new MockI2CController(file, ctxType, logger));
#else
				error(logger, "Cannot use mock I2C context. Not built in this version");
				return nullptr;
#endif
			default:
				error(logger, "Invalid I2C backend!");
				return nullptr;
		}
	}

	uint8_t PCF8591::Read() {
		uint8_t b;
		m_Ctrl->SelectDevice(m_Address);
		ssize_t result = m_Ctrl->Read(&b, 1);
		if (result < 0)
			throw core::InternalError(fmt::format("PCF8591::Read(): Read failed ({})", -result));
		else if (result != 1)
			throw core::InternalError("PCF8591::Read(): Read failed (Unknown Error)");
		return b;
	}

	void PCF8591::SendControl(int channel, bool autoinc, InputMode mode, bool outputEnable) {
		m_CurChannel = channel;
		m_AutoInc = autoinc;
		m_InputMode = mode;
		m_OutputEnable = outputEnable;
		m_Ctrl->SelectDevice(m_Address);
		uint8_t b = ComposeControlByte(m_CurChannel, m_AutoInc, m_InputMode, m_OutputEnable);
		ssize_t result = m_Ctrl->Write(&b, 1);
		if (result < 0)
			throw core::InternalError(fmt::format("PCF8591::SendControl(): Write failed ({})", -result));
		else if (result != 1)
			throw core::InternalError("PCF8591::SendControl(): Write failed (Unknown Error)");
		Read();
	}

	void PCF8591::SelectChannel(int channel) {
		if (m_CurChannel == channel)
			return;
		SendControl(channel, m_AutoInc, m_InputMode, m_OutputEnable);
	}
}
}

