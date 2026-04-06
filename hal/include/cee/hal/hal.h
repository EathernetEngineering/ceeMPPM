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

#ifndef CEE_HAL_HAL_H_
#define CEE_HAL_HAL_H_

#include <spdlog/spdlog.h>

enum HalGfxBackend {
	HAL_GFX_BACKEND_NONE = 0,
	HAL_GFX_BACKEND_DRM  = 1,
	HAL_GFX_BACKEND_X11  = 2,
	
	HAL_GFX_BACKEND_ENUM_MAX
};
typedef enum HalGfxBackend hal_gfx_backend_t;

enum HalI2CBackend {
	HAL_I2C_BACKEND_NONE = 0,
	HAL_I2C_BACKEND_HW  = 1,
	HAL_I2C_BACKEND_MOCK  = 2,
	
	HAL_I2C_BACKEND_ENUM_MAX
};
typedef enum HalI2CBackend hal_i2c_backend_t;

namespace cee {
namespace hal {
	int Init(void);
	int Shutdown(void);

	int SetGfxBackend(hal_gfx_backend_t backend);
	hal_gfx_backend_t GetGfxBackend(void);
	int SetI2CBackend(hal_i2c_backend_t backend);
	hal_i2c_backend_t GetI2CBackend(void);

	void InitLogger();
	void ShutdownLogger();
	std::shared_ptr<spdlog::logger> GetLogger();
}
}

#endif

