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

#include <display.h>
#include <log.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX_DRM_DEVICES 64

static int HALGetResources(struct display *dpy)
{
	dpy->resources = drmModeGetResources(dpy->fd);
	if (!dpy->resources) {
		CEE_WARN("drmModeGetResources failed");
		return -1;
	}
	return 0;
}

static int FindCrtcForEncoder(struct display *dpy, const drmModeEncoder *encoder)
{
	for (int i = 0; i < dpy->resources->count_crtcs; i++) {
		const uint32_t crtcMask = 1 << i;
		const uint32_t crtcId = dpy->resources->crtcs[i];

		if (encoder->possible_crtcs & crtcMask) {
			return crtcId;
		}
	}

	return -1;
}

static int FindCrtcForConnector(struct display *dpy, const drmModeConnector *connector)
{
	for (int i = 0; i < connector->count_encoders; i++) {
		const uint32_t encoderId = connector->encoders[i];
		drmModeEncoder *encoder = drmModeGetEncoder(dpy->fd, encoderId);

		if (encoder) {
			const int32_t crtcId = FindCrtcForEncoder(dpy, encoder);
			drmModeFreeEncoder(encoder);

			if (crtcId != 0) {
				return crtcId;
			}
		}
	}

	return -1;
}

static void HALDrmPageFlipHandler(int fd, uint32_t frame, uint32_t sec, uint32_t usec, void* data) {
	(void)fd, (void)frame, (void)sec, (void)usec; // Unused parametes
	
	*((int *)data) = 0;
}

int HALFindDrmDevice(struct display *dpy)
{
	drmDevicePtr devices[MAX_DRM_DEVICES] = { NULL };
	int countDevices;
	dpy->fd = -1;

	countDevices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
	if (countDevices < 0) {
		CEE_ERROR("drmGetDevices2 failed: %s", strerror(-countDevices));
		return -1;
	}

	for (int i = 0; i < countDevices; i++) {
		drmDevicePtr device = devices[i];
		int ret;

		if (!(device->available_nodes & (1 << DRM_NODE_PRIMARY))) {
			continue;
		}

		dpy->fd = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR);
		if (dpy->fd < 0) {
			continue;
		}
		ret = HALGetResources(dpy);
		if (ret == 0) {
			CEE_DEBUG("Using device %s", device->nodes[DRM_NODE_PRIMARY]);
			break;
		}
		close(dpy->fd);
		dpy->fd = -1;
	}
	drmFreeDevices(devices, countDevices);

	if (dpy->fd < 0) {
		CEE_ERROR("No drm device found!");
		return dpy->fd;
	}

	dpy->eventContext.version = 2;
	dpy->eventContext.page_flip_handler = HALDrmPageFlipHandler;

	return 0;
}

int HALFindConnector(struct display *dpy)
{
	if (dpy->connectorId >= 0) {
		if (dpy->connectorId >= dpy->resources->count_connectors) {
			CEE_ERROR("Provided connector ID invlalid. Given %i. connector count: %i", dpy->connectorId, dpy->resources->count_connectors);
			return -1;
		}

		dpy->connector = drmModeGetConnector(dpy->fd, dpy->resources->connectors[dpy->connectorId]);
		if (dpy->connector == NULL) {
			CEE_ERROR("Provided dpy->connector ID invlalid");
			return -1;
		}
		if (dpy->connector->connection != DRM_MODE_CONNECTED) {
			CEE_ERROR("Provided dpy->connector ID is not connected");
			drmModeFreeConnector(dpy->connector);
			return -1;
		}
		return 0;
	}

	for (int i = 0; i < dpy->resources->count_connectors; i++) {
		dpy->connector = drmModeGetConnector(dpy->fd, dpy->resources->connectors[i]);
		if (dpy->connector == NULL) {
			continue;
		}
		if (dpy->connector->connection == DRM_MODE_CONNECTED) {
			break;
		}
		drmModeFreeConnector(dpy->connector);
		dpy->connector = NULL;
	}
	if (dpy->connector == NULL)
		return -1;
	return 0;
}

int HALChoooseConnectorMode(struct display *dpy)
{
	int area;
	for (int i = 0; i < dpy->connector->count_modes; i++) {
		drmModeModeInfo *currentMode = &dpy->connector->modes[i];

		if (currentMode->type & DRM_MODE_TYPE_PREFERRED) {
			dpy->connectorMode = currentMode;
			break;
		}
		int currentArea = currentMode->hdisplay * currentMode->vdisplay;
		if (currentArea > area) {
			dpy->connectorMode = currentMode;
			area = currentArea;
		}
	}

	if (!dpy->connectorMode) {
		CEE_ERROR("could not find mode!");
		return -1;
	}

	dpy->width = dpy->connectorMode->hdisplay;
	dpy->height = dpy->connectorMode->vdisplay;

	for (int i = 0; i < dpy->resources->count_encoders; i++) {
		dpy->encoder = drmModeGetEncoder(dpy->fd, dpy->resources->encoders[i]);
		if (dpy->encoder->encoder_id == dpy->connector->encoder_id) {
			break;
		}
		drmModeFreeEncoder(dpy->encoder);
		dpy->encoder = NULL;
	}

	if (dpy->encoder) {
		dpy->crtcId = dpy->encoder->crtc_id;
	} else {
		int crtcId = FindCrtcForConnector(dpy, dpy->connector);
		if (crtcId == -1) {
			CEE_ERROR("No crtc found!");
			return -1;
		}
		dpy->crtcId = crtcId;
	}

	for (int i = 0; i < dpy->resources->count_crtcs; i++) {
		if (dpy->resources->crtcs[i] == dpy->crtcId) {
			dpy->crtcIndex = i;
			break;
		}
	}
	drmModeFreeResources(dpy->resources);

	dpy->connectorId = dpy->connector->connector_id;

	return 0;
}

int HALDisplayPageFlip(struct display *dpy, void *fbo, const uint32_t fboId)
{
	int waitingForFlip = 1, result = -1;
	fd_set fds;
	if ((result = drmModePageFlip(dpy->fd, dpy->crtcId, fboId, DRM_MODE_PAGE_FLIP_EVENT, &waitingForFlip))) {
		CEE_DEBUG("drmModePageFlip: %s", strerror(errno));
		return result;
	}

	while (waitingForFlip) {
		FD_ZERO(&fds);
		FD_SET(dpy->fd, &fds);

		result = select(dpy->fd + 1, &fds, NULL, NULL, NULL);
		if (result < 0) {
		} else if (result == 0) {
			CEE_WARN("select timeout");
			return 0;
		} else {
			drmHandleEvent(dpy->fd, &dpy->eventContext);
		}
	}

	return 0;
}

