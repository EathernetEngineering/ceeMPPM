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

#ifndef CEE_HAL_GFX_GLX_H_
#define CEE_HAL_GFX_GLX_H_

#include <xcb/xcb.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct HALGfx_glx;

struct HALGfx_glx *HALGfxGlXCreate(void);
int HALGfxGlXInit(struct HALGfx_glx *gfx);
int HALGfxGlXShutdown(struct HALGfx_glx *gfx);
void HALGfxGlXDestroy(struct HALGfx_glx *gfx);

const char *HALGfxGlXGetVersionString(const struct HALGfx_glx *gfx);

int HALGfxGlXGetWidth(const struct HALGfx_glx *gfx);
int HALGfxGlXGetHeight(const struct HALGfx_glx *gfx);
int HALGfxGlXPageFlip(struct HALGfx_glx *gfx);

int HALGfxGlXCreateWindow(struct HALGfx_glx *gfx, int width, int height, const char *title);

#if defined(__cplusplus)
}
#endif

#endif

