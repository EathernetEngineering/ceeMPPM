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

#ifndef CEE_HAL_GFX_UTIL_H_
#define CEE_HAL_GFX_UTIL_H_

#include <glad/egl.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef const char *(*PFNGetPlatformVersionString)(const void *platformGfx);
typedef int(*PFNGetPlatformWidth)(const void *platformGfx);
typedef int(*PFNGetPlatformHeight)(const void *platformGfx);
typedef int(*PFNGetPlatformPageFlip)(const void *platformGfx);

#if defined(__cplusplus)
}
#endif

#endif

