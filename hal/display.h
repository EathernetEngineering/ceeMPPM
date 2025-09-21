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

#ifndef CEE_HAL_DISPLAY_H_
#define CEE_HAL_DISPLAY_H_

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#include <stdint.h>

struct display {
	int fd;
	drmModeRes *resources;
	drmModeConnector *connector;
	drmModeEncoder *encoder;
	drmModeModeInfo *connectorMode;
	uint32_t connectorId;
	uint32_t crtcIndex;
	uint32_t crtcId;
	drmEventContext eventContext;

	int width, height;
};

int HALFindDrmDevice(struct display *dpy);
int HALFindConnector(struct display *dpy);
int HALChoooseConnectorMode(struct display *dpy);
int HALDisplayPageFlip(struct display *dpy, void *fbo, uint32_t fboId);

#endif

