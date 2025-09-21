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

#include <cee/hal/gfx.h>
#include <cee/hal/hal.h>
#include <gfx_drm.h>
#include <gfx_egl_x.h>
#include <gfx_glx.h>
#include <log.h>
#include <util.h>
#include <config.h>

#include <stddef.h>
#include <stdlib.h>

struct HALGfx {
	void* platformGfx;
	PFNGetPlatformVersionString pfnGetVersionString;
	PFNGetPlatformWidth pfnGetWidth;
	PFNGetPlatformHeight pfnGetHeight;
	PFNGetPlatformPageFlip pfnPageFlip;
};

static int SetPfns(struct HALGfx *gfx)
{
	switch (HALGetGfxBackend()) {
		case HAL_GFX_BACKEND_DRM:
#if BUILD_HAL_DRM
			{	
				gfx->pfnGetVersionString   = (PFNGetPlatformVersionString)HALGfxDRMGetVersionString;
				gfx->pfnGetWidth   = (PFNGetPlatformWidth)HALGfxDRMGetWidth;
				gfx->pfnGetHeight  = (PFNGetPlatformHeight)HALGfxDRMGetHeight;
				gfx->pfnPageFlip   = (PFNGetPlatformPageFlip)HALGfxDRMPageFlip;
			}	
			return 0;
#else
			CEE_ERROR("Graphics backend not build!");
			return -1;
#endif

		case HAL_GFX_BACKEND_EGL_X:
#if BUILD_HAL_EGL_X11
			{
				gfx->pfnGetVersionString   = (PFNGetPlatformVersionString)HALGfxEglXGetVersionString;
				gfx->pfnGetWidth   = (PFNGetPlatformWidth)HALGfxEglXGetWidth;
				gfx->pfnGetHeight  = (PFNGetPlatformHeight)HALGfxEglXGetHeight;
				gfx->pfnPageFlip   = (PFNGetPlatformPageFlip)HALGfxEglXPageFlip;
			}	
			return 0;
#else
			CEE_ERROR("Graphics backend not build!");
			return -1;
#endif

		case HAL_GFX_BACKEND_GLX:
#if BUILD_HAL_GLX
			{
				gfx->pfnGetVersionString   = (PFNGetPlatformVersionString)HALGfxGlXGetVersionString;
				gfx->pfnGetWidth   = (PFNGetPlatformWidth)HALGfxGlXGetWidth;
				gfx->pfnGetHeight  = (PFNGetPlatformHeight)HALGfxGlXGetHeight;
				gfx->pfnPageFlip   = (PFNGetPlatformPageFlip)HALGfxGlXPageFlip;
			}	
			return 0;
#else
			CEE_ERROR("Graphics backend not build!");
			return -1;
#endif

		default:
			CEE_ERROR("Invalid graphics backend!");
			return -1;
	}
}

struct HALGfx *HALGfxCreate(void)
{
	return calloc(1, sizeof(struct HALGfx));
}

int HALGfxInit(struct HALGfx *gfx)
{
	int c;
	if ((c = SetPfns(gfx)))
		return c;

	switch (HALGetGfxBackend()) {
		case HAL_GFX_BACKEND_DRM:
#if BUILD_HAL_DRM
			{
				if (!(gfx->platformGfx = HALGfxDRMCreate()))
					return -1;
				c = HALGfxDRMInit(gfx->platformGfx);
			}
			return c;
#else
			return -1;
#endif

		case HAL_GFX_BACKEND_EGL_X:
#if BUILD_HAL_EGL_X11
			{
				if (!(gfx->platformGfx = HALGfxEglXCreate()))
					return -1;
				if ((c = HALGfxEglXInit(gfx->platformGfx)))
					return c;
				c = HALGfxEglXCreateWindow(gfx->platformGfx, 1280, 720, "Cardiac Monitor");
			}
			return c;
#else
			return -1;
#endif

		case HAL_GFX_BACKEND_GLX:
#if BUILD_HAL_GLX
			{
				if (!(gfx->platformGfx = HALGfxGlXCreate()))
					return -1;
				if ((c = HALGfxGlXInit(gfx->platformGfx)))
					return c;
				c = HALGfxGlXCreateWindow(gfx->platformGfx, 1280, 720, "Cardiac Monitor");
			}
			return c;
#else
			return -1;
#endif

		default:
			CEE_ERROR("Invalid graphics backend!");
			return 0;
	}
}

int HALGfxShutdown(struct HALGfx *gfx)
{
	return 0;
}

void HALGfxDestroy(struct HALGfx *gfx)
{
	free(gfx);
}

int HALGfxGetWidth(const struct HALGfx *gfx)
{
	if (gfx == NULL) {
		return -1;
	}

	if (gfx->pfnGetWidth)
		return gfx->pfnGetWidth(gfx->platformGfx);

	CEE_ERROR("Call to HALGfxGetWidth without bound function");
	return -1;
}

int HALGfxGetHeight(const struct HALGfx *gfx)
{
	if (gfx == NULL) {
		return -1;
	}

	if (gfx->pfnGetHeight)
		return gfx->pfnGetHeight(gfx->platformGfx);

	CEE_ERROR("Call to HALGfxGetHeight without bound function");
	return -1;
}

int HALGfxPageFlip(struct HALGfx *gfx)
{
	if (gfx == NULL) {
		return -1;
	}

	if (gfx->pfnPageFlip)
		return gfx->pfnPageFlip(gfx->platformGfx);

	CEE_ERROR("Call to HALGfxPageFlip without bound function");
	return -1;
}

const char *HALGfxGetVersionString(struct HALGfx *gfx)
{
	if (gfx == NULL) {
		return NULL;
	}

	if (gfx->pfnGetVersionString)
		return gfx->pfnGetVersionString(gfx->platformGfx);

	CEE_ERROR("Call to HALGfxGetVersionString without bound function");
	return NULL;
}

