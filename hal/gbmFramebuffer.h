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

#ifndef CEE_FRAMEBUFFER_H_
#define CEE_FRAMEBUFFER_H_

#include <gbm.h>

#include <stdint.h>

struct gbmFb {
	struct gbm_device *gbmDevice;;
	struct gbm_surface *gbmSurface;;
	struct gbm_bo *gbmFbo;
	struct gbm_bo *gbmBbo;
	uint32_t gbmFboId;

	int width, height;
	uint32_t format;
};

int HALCreateGbmDevice(struct gbmFb *fb, int drmFd);
int HALDestroyGbmDevice(struct gbmFb *fb);

int HALCreateGbmSurface(struct gbmFb *fb, int width, int height, uint32_t format);
int HALDestryGbmSurface(struct gbmFb *fb);

int HALSetupGbmBo(struct gbmFb *fb);
int HALDestryGbmBo(struct gbmFb *fb);

int HALGbmPageFlip(struct gbmFb *fb);
void HALReleaseBuffer(struct gbmFb *fb);

#endif


