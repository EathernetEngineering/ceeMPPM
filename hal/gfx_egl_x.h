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

#ifndef CEE_HAL_GFX_EGL_X_H_
#define CEE_HAL_GFX_EGL_X_H_

#include <cee/hal/gfx.h>
#include <xcb/xcb.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct HALGfx_egl_x;

struct HALGfx_egl_x *HALGfxEglXCreate(void);
int HALGfxEglXInit(struct HALGfx_egl_x *gfx);
int HALGfxEglXShutdown(struct HALGfx_egl_x *gfx);
void HALGfxEglXDestroy(struct HALGfx_egl_x *gfx);

const char *HALGfxEglXGetVersionString(const struct HALGfx_egl_x *gfx);

int HALGfxEglXGetWidth(const struct HALGfx_egl_x *gfx);
int HALGfxEglXGetHeight(const struct HALGfx_egl_x *gfx);
int HALGfxEglXPageFlip(struct HALGfx_egl_x *gfx);

int HALGfxEglXCreateWindow(struct HALGfx_egl_x *gfx, int width, int height, const char *title);

#if defined(__cplusplus)
}
#endif

#endif

