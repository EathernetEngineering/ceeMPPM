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

#ifndef CEE_PLATFORM_I2C_HW_H_
#define CEE_PLATFORM_I2C_HW_H_

#include <cee/platform/i2c.h>

namespace cee {
namespace platform {
	class HwI2CController : public I2CController {
	protected:
		HwI2CController(const std::string &file, I2CContextType ctxType, Logger logger);

	public:
		virtual ~HwI2CController();

		virtual void SelectDevice(uint8_t address) override;

		virtual ssize_t Read(void *data, ssize_t count) override;
		virtual ssize_t Write(const void *data, ssize_t count) override;

	private:
		int m_Fd;
		uint8_t m_PrevAddress;

	public:
		friend std::shared_ptr<I2CController> I2CController::Create(const std::string &file,
				I2CContextType ctxType,
				Logger logger);
	};
}
}

#endif
