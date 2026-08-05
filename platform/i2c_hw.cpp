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

#include <i2c_hw.h>

#include <format>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

namespace cee {
namespace platform {
	HwI2CController::HwI2CController(const std::string &file, I2CContextType ctxType, Logger logger)
	 : I2CController(ctxType, logger) {
		m_Fd = open(file.c_str(), O_RDWR);
		if (m_Fd < 0) {
			throw core::InternalError(fmt::format("I2CController(): Failed to open I2C device {}", file));
		}
	}

	HwI2CController::~HwI2CController() {
		if (m_Fd > 0)
			close(m_Fd);
	}

	void HwI2CController::SelectDevice(uint8_t address) {
		int result = ioctl(m_Fd, I2C_SLAVE, address);
		if (result < 0)
			throw core::InternalError(fmt::format("Failed to select address {:02X} ({})", address, result));
		m_PrevAddress = address;
	}

	ssize_t HwI2CController::Read(void *data, ssize_t count) {
		return read(m_Fd, data, count);
	}

	ssize_t HwI2CController::Write(const void *data, ssize_t count) {
		return write(m_Fd, data, count);
	}
}
}
