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

#ifndef CEE_PLATFORM_I2C_H_
#define CEE_PLATFORM_I2C_H_

#include <cee/core/except.h>
#include <cee/core/log.h>

#include <cstdint>
#include <memory>
#include <string>

namespace cee {
namespace platform {
	enum class I2CContextType {
		PLATFORM_I2C_CONTEXT_NONE = 0,
		PLATFORM_I2C_CONTEXT_HW  = 1,
		PLATFORM_I2C_CONTEXT_MOCK  = 2,
		
		PLATFORM_I2C_CONTEXT_ENUM_MAX
	};

	class I2CController {
	protected:
		I2CController(I2CContextType ctxtype, Logger logger)
		 : m_CtxType(ctxtype), m_Logger(logger) {
		}

	public:
		virtual ~I2CController() {}

		virtual void SelectDevice(uint8_t address) = 0;

		virtual ssize_t Read(void *data, ssize_t count) = 0;
		virtual ssize_t Write(const void *data, ssize_t count) = 0;

		I2CContextType GetContextType() const { return m_CtxType; }

		static std::shared_ptr<I2CController> Create(const std::string &file,
				I2CContextType ctxType,
				Logger logger = nullptr);

	protected:
		Logger &logger() { return m_Logger; }

	private:
		I2CContextType m_CtxType;
		Logger m_Logger;
	};

	class PCF8591 {
	public:
		enum class InputMode : uint8_t {
			SINGLE_ENDED = 0,
			THREE_DIFFERENTIAL,
			ONE_DIFFERENTIAL,
			TWO_DIFFERENTIAL,
		};

	public:
		PCF8591(std::shared_ptr<I2CController> ctrl, uint8_t addr) : m_Ctrl(ctrl), m_Address(addr) {}
		~PCF8591() {}

		uint8_t Read();
		void SendControl(int channel, bool autoinc, InputMode mode, bool outputEnable);
		void SelectChannel(int channel);

	private:
		static constexpr uint8_t ComposeControlByte(uint8_t channel,
				bool autoinc, InputMode mode, bool outputEnable) {
			uint8_t b = 0;
			if (channel > 3)
				throw core::InvalidParameter(
						fmt::format("PCF8591::ComposeControlByte(): Channel {} doesn't exist",
							channel));
			b |= channel;
			if (autoinc)
				b |= 1 << 2;
			switch (mode) {
				case InputMode::SINGLE_ENDED:                        break;
				case InputMode::THREE_DIFFERENTIAL: b |= 0b00010000; break;
				case InputMode::ONE_DIFFERENTIAL:   b |= 0b00100000; break;
				case InputMode::TWO_DIFFERENTIAL:   b |= 0b00110000; break;
				default:
					throw core::InvalidParameter("PCF8591::ComposeControlByte(): Invalid mode");
			}
			if (outputEnable)
				b |= 0b01000000;
			return b;
		}

	private:
		std::shared_ptr<I2CController> m_Ctrl;
		uint8_t m_Address;
		int m_CurChannel;
		bool m_AutoInc;
		InputMode m_InputMode;
		bool m_OutputEnable;
	};
}
}

#endif

