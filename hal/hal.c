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

#include <cee/hal/hal.h>
#include <log.h>

static hal_gfx_backend_t g_GfxBackend = HAL_GFX_BACKEND_NONE;
static hal_i2c_backend_t g_I2CBackend = HAL_I2C_BACKEND_NONE;
static int g_Initialized = 0;

int HALInit(void)
{
	if (g_Initialized) {
		return -1;
	}
	if (g_GfxBackend == HAL_GFX_BACKEND_NONE) {
		CEE_CRITICAL("HAL graphics backend not set!!!");
		return -1;
	}
	if (g_I2CBackend == HAL_I2C_BACKEND_NONE) {
		CEE_CRITICAL("HAL I2C backend not set!!!");
		return -1;
	}

	g_Initialized = 1;
	return 0;
}

int HALShutdown(void)
{
	return 0;
}

int HALSetGfxBackend(hal_gfx_backend_t backend)
{
	if (g_Initialized) {
		return -1;
	}
	if (backend > HAL_GFX_BACKEND_NONE && backend < HAL_GFX_BACKEND_ENUM_MAX) {
		g_GfxBackend = backend;
		return 0;
	}
	return -1;
}

hal_gfx_backend_t HALGetGfxBackend(void)
{
	return g_GfxBackend;
}

int HALSetI2CBackend(hal_i2c_backend_t backend)
{
	if (g_Initialized) {
		return -1;
	}
	if (backend > HAL_I2C_BACKEND_NONE && backend < HAL_I2C_BACKEND_ENUM_MAX) {
		g_I2CBackend = backend;
		return 0;
	}
	return -1;
}

hal_i2c_backend_t HALGetI2CBackend(void)
{
	return g_I2CBackend;
}

