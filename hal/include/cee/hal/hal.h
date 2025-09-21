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

#if defined(__cplusplus)
extern "C" {
#endif

enum HalGfxBackend {
	HAL_GFX_BACKEND_NONE = 0,
	HAL_GFX_BACKEND_DRM  = 1,
	HAL_GFX_BACKEND_EGL_X  = 2,
	HAL_GFX_BACKEND_GLX  = 3,
	
	HAL_GFX_BACKEND_ENUM_MAX
};
typedef enum HalGfxBackend hal_gfx_backend_t;

enum HalI2CBackend {
	HAL_I2C_BACKEND_NONE = 0,
	HAL_I2C_BACKEND_I2C  = 1,
	HAL_I2C_BACKEND_SIM  = 2,
	
	HAL_I2C_BACKEND_ENUM_MAX
};
typedef enum HalI2CBackend hal_i2c_backend_t;

int HALInit(void);
int HALShutdown(void);

int HALSetGfxBackend(hal_gfx_backend_t backend);
hal_gfx_backend_t HALGetGfxBackend(void);
int HALSetI2CBackend(hal_i2c_backend_t backend);
hal_i2c_backend_t HALGetI2CBackend(void);

#if defined(__cplusplus)
}
#endif

#endif

