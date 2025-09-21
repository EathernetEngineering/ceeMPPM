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

#ifndef CEE_HAL_GFX_DRM_H_
#define CEE_HAL_GFX_DRM_H_

#include <cee/hal/gfx.h>
#include <config.h>

#include <glad/egl.h>
#if BUILD_GLES
#include <glad/gles2.h>
#elif BUILD_GL
#include <glad/gl.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

struct HALGfx_DRM;

struct HALGfx_DRM *HALGfxDRMCreate(void);
int HALGfxDRMInit(struct HALGfx_DRM *gfx);
int HALGfxDRMShutdown(struct HALGfx_DRM *gfx);
void HALGfxDRMDestroy(struct HALGfx_DRM *gfx);

const char *HALGfxDRMGetVersionString(const struct HALGfx_DRM *gfx);

int HALGfxDRMGetWidth(const struct HALGfx_DRM *gfx);
int HALGfxDRMGetHeight(const struct HALGfx_DRM *gfx);
int HALGfxDRMPageFlip(struct HALGfx_DRM *gfx);

#if defined(__cplusplus)
}
#endif

#endif

