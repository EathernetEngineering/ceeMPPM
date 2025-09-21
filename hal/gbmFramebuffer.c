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

#include <gbmFramebuffer.h>
#include <log.h>

#include <drm.h>
#include <drm_mode.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <gbm.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

static struct gbm_bo *g_OldFbo;

static void destroyUserDataCallback(struct gbm_bo *bo, void *userData)
{
	struct gbmFbInfo {
		struct gbm_bo *bo;
		uint32_t id;
	} *fbInfo;
	(void)bo; // Unused parameter
	free(userData);
}

static int getDrmModeId(struct gbmFb *fb)
{
	struct gbmFbInfo {
		struct gbm_bo *bo;
		uint32_t id;
	} *fbInfo;
	int fd;
	fbInfo = gbm_bo_get_user_data(fb->gbmBbo);
	if (fbInfo) {
		fb->gbmFbo = fbInfo->bo;
		fb->gbmFboId = fbInfo->id;

		return 0;
	}
	fd = gbm_device_get_fd(fb->gbmDevice);

	fbInfo = calloc(1, sizeof(*fbInfo));
	if (!fbInfo)
		return -1;
	fb->gbmFbo = fb->gbmBbo;
	fb->gbmFboId = 0;


	uint32_t strides[4] = { 0 };
	uint32_t offsets[4] = { 0 };
	uint32_t handles[4] = { 0 };
	uint32_t format = 0;
	uint32_t width = 0, height = 0;
	uint32_t flags = 0;
	int32_t result = -1;

	uint64_t modifiers[4] = { 0 };
	width = gbm_bo_get_width(fb->gbmBbo);
	height = gbm_bo_get_height(fb->gbmBbo);
	format = gbm_bo_get_format(fb->gbmBbo);
	modifiers[0] = gbm_bo_get_modifier(fb->gbmBbo);
	const int planeCount = gbm_bo_get_plane_count(fb->gbmBbo);
	for (uint32_t i = 0; i < planeCount; i++) {
		strides[i] = gbm_bo_get_stride_for_plane(fb->gbmBbo, i);
		offsets[i] = gbm_bo_get_offset(fb->gbmBbo, i);
		handles[i] = gbm_bo_get_handle_for_plane(fb->gbmBbo, i).u32;
		modifiers[i] = modifiers[0];
	}
	if (modifiers[0] && (modifiers[0] != DRM_FORMAT_MOD_INVALID)) { 
		flags = DRM_MODE_FB_MODIFIERS;
			CEE_DEBUG("drmModeAddFB2WithModifiers with modifiers: %llu", modifiers[0]);
	}

	result = drmModeAddFB2WithModifiers(fd,
							width, height,
							format,
							handles,
							strides, offsets,
							modifiers,
							&fb->gbmFboId, flags);

	if (result)  {
		if (flags)
			CEE_DEBUG("drmModeAddFB2WithModifiers failed: %s", strerror(errno));

		handles[0] = gbm_bo_get_handle(fb->gbmBbo).u32;
		strides[0] = gbm_bo_get_stride(fb->gbmBbo);
		memset(&strides[1], 0, sizeof(uint32_t)*3);
		memset(&offsets[0], 0, sizeof(uint32_t)*4);
		memset(&handles[1], 0, sizeof(uint32_t)*3);

		result = drmModeAddFB2(fd,
						  fb->width, fb->height,
						  format,
						  handles,
						  strides, offsets,
						  &fb->gbmFboId, 0);

		if (result) {
			CEE_ERROR("Fallback failed: drmModeAddFB2: %s", strerror(errno));
			CEE_DEBUG("fd = %i, width = %i, height = %i, format = %u, handle = %p", fd, fb->width, fb->height, format, handles[0]);
			CEE_DEBUG("stride = %u, offset = %u, drmBoId = %u, flags = 0", strides[0], offsets[0], fb->gbmFboId);
			free(fbInfo);
			return result;
		}
		fbInfo->bo = fb->gbmBbo;
		fbInfo->id = fb->gbmFboId;

		gbm_bo_set_user_data(fb->gbmBbo, fbInfo, destroyUserDataCallback);
	}
	return 0;
}

int HALCreateGbmDevice(struct gbmFb *fb, int drmFd)
{
	if ((fb->gbmDevice = gbm_create_device(drmFd)))
		return 0;
	
	return -1;
}

int HALDestroyGbmDevice(struct gbmFb *fb)
{
	gbm_device_destroy(fb->gbmDevice);
	fb->gbmDevice = NULL;

	return 0;
}

int HALCreateGbmSurface(struct gbmFb *fb, int width, int height, uint32_t format)
{
	fb->width = width;
	fb->height = height;
	fb->format = format;

	uint64_t modifiers[] = { DRM_FORMAT_MOD_LINEAR };

	fb->gbmSurface = gbm_surface_create_with_modifiers2(
								   fb->gbmDevice,
								   fb->width,
								   fb->height,
								   fb->format,
								   modifiers, (sizeof(modifiers)/sizeof(modifiers[0])),
								   GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

	if (fb->gbmSurface)
		return 0;

	g_OldFbo = NULL;

	return -1;
}

int HALDestryGbmSurface(struct gbmFb *fb)
{
	gbm_surface_destroy(fb->gbmSurface);
	return 0;
}

int HALSetupGbmBo(struct gbmFb *fb)
{
	fb->gbmBbo = gbm_surface_lock_front_buffer(fb->gbmSurface);
	getDrmModeId(fb);
	return 0;
}

int HALDestryGbmBo(struct gbmFb *fb)
{
	return 0;
}

int HALGbmPageFlip(struct gbmFb *fb)
{
	fb->gbmBbo = gbm_surface_lock_front_buffer(fb->gbmSurface);
	g_OldFbo = fb->gbmFbo;
	getDrmModeId(fb);

	return 0;
}

void HALReleaseBuffer(struct gbmFb *fb)
{
	gbm_surface_release_buffer(fb->gbmSurface, g_OldFbo);
}

