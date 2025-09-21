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

#include <gfx_drm.h>
#include <log.h>
#include <display.h>
#include <gbmFramebuffer.h>
#include <config.h>

#include <glad/egl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#if BUILD_GLES
#include <glad/gles2.h>
#elif BUILD_GL
#include <glad/gl.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct HALGfx_DRM {
	EGLDisplay eglDisplay;
	EGLConfig eglConfig;
	EGLContext eglContext;
	EGLSurface eglSurface;
	const char *versionString;
	int width;
	int height;

	struct display dpy;
	struct gbmFb fb;
};

static int MatchEglConfigToVisual(const struct HALGfx_DRM *gfx, EGLConfig configs[], int count) {
	for (int i = 0; i < count; i++) {
		EGLint id;
		if (!eglGetConfigAttrib(gfx->eglDisplay, configs[i], EGL_NATIVE_VISUAL_ID, &id))
			continue;

		if (id == gfx->fb.format)
			return i;
	}
	return -1;
}

static int ChooseEglConfig(struct HALGfx_DRM *gfx, const EGLint *attribs) {
	EGLint count = 0;
	EGLint matched = 0;
	EGLConfig *configs;
	int configIndex = -1;

	if (!eglGetConfigs(gfx->eglDisplay, NULL, 0, &count)) {
		CEE_ERROR("No EGL configs to choose from");
		return -1;
	}
	configs = (EGLConfig *)(calloc(count, sizeof(*configs)));
	if (configs == NULL) {
		CEE_ERROR("failed to allocate configs");
		return -1;
	}

	if (!eglChooseConfig(gfx->eglDisplay, attribs, configs, count, &matched) || !matched) {
		CEE_ERROR("No EGL configs with approprriate attributes");
		free(configs);
		return -1;
	}

	if (!gfx->fb.format)
		configIndex = 0;
	
	if (configIndex == -1)
		configIndex = MatchEglConfigToVisual(gfx, configs, count);
	
	if (configIndex != -1)
		gfx->eglConfig = configs[configIndex];
	free(configs);
	if (configIndex == -1) {
		return -1;
	}
	return 0;
}

struct HALGfx_DRM *HALGfxDRMCreate(void)
{
	return calloc(1, sizeof(struct HALGfx_DRM));
}

int HALGfxDRMInit(struct HALGfx_DRM *gfx)
{
	if (gfx == NULL) {
		return -1;
	}

	EGLBoolean result = EGL_FALSE;
	GLenum ec;
	EGLint major, minor;

	if (HALFindDrmDevice(&gfx->dpy)) {
		CEE_ERROR("Failed to open DRM device");
		return gfx->dpy.fd;
	}

	if (HALFindConnector(&gfx->dpy)) {
		CEE_ERROR("Failed to open DRM device connector");
		return -1;
	}

	if (HALChoooseConnectorMode(&gfx->dpy)) {
		CEE_ERROR("Failed to choose mode for connector");
		return -1;
	} else {
		CEE_DEBUG("Chose connector 0x%X, %ix%i@%iHz",
			gfx->dpy.connectorId,
			gfx->dpy.connectorMode->hdisplay,
			gfx->dpy.connectorMode->vdisplay,
			gfx->dpy.connectorMode->vrefresh);
	}

	if (HALCreateGbmDevice(&gfx->fb, gfx->dpy.fd)) {
		CEE_ERROR("Failed to create gbm device");
		return -1;
	}

	if (HALCreateGbmSurface(&gfx->fb, gfx->dpy.width, gfx->dpy.height, GBM_FORMAT_XRGB8888)) {
		CEE_ERROR("Failed to create gbm device");
		return -1;
	}

	gladLoaderLoadEGL(NULL);

	PFNEGLGETPLATFORMDISPLAYPROC pfn_eglGetPlatfromDisply =
		(void *)eglGetProcAddress("eglGetPlatformDisplay");
	gfx->eglDisplay = EGL_NO_DISPLAY;

	if (pfn_eglGetPlatfromDisply) {
		gfx->eglDisplay = pfn_eglGetPlatfromDisply(EGL_PLATFORM_GBM_KHR, (void *)gfx->fb.gbmDevice, NULL);
	} else {
		CEE_DEBUG("Failed to load eglGetPlatformDisplay");

		PFNEGLGETPLATFORMDISPLAYEXTPROC pfn_eglGetPlatformDisplayEXT =
			(void *)eglGetProcAddress("eglGetPlatformDisplayEXT");

		if (pfn_eglGetPlatformDisplayEXT) {
			gfx->eglDisplay = pfn_eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, (void *)gfx->fb.gbmDevice, NULL);
		} else {
			CEE_ERROR("Failed to load eglGetPlatformDisplayEXT");

			return -1;
		}
	}

	if (gfx->eglDisplay == EGL_NO_DISPLAY) {
		CEE_ERROR("Could not get platform EGLDisplay");
		return -1;
	}

	if (!(result = eglInitialize(gfx->eglDisplay, &major, &minor))) {
		CEE_ERROR("Failed to initialize egl");
		return -1;
	}

	gladLoaderLoadEGL(gfx->eglDisplay);
	CEE_DEBUG("Loaded EGL %i.%i:", major, minor);
	CEE_DEBUG("     VENDOR: %s", eglQueryString(gfx->eglDisplay, EGL_VENDOR));
	CEE_DEBUG("    DISPLAY: %p", gfx->eglDisplay);

#if BUILD_GLES
	if (!(result = eglBindAPI(EGL_OPENGL_ES_API))) {
#elif BUILD_GL
	if (!(result = eglBindAPI(EGL_OPENGL_API))) {
#else
	if (1) {
#endif
		CEE_ERROR("Failed to bind egl openGL");
		return -1;
	}

	static const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	EGLint configAttribs[] = {
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	if (ChooseEglConfig(gfx, configAttribs)) {
		CEE_ERROR("Failed to choose config");
		return -1;
	}

	gfx->eglContext = eglCreateContext(gfx->eglDisplay, gfx->eglConfig, EGL_NO_CONTEXT, contextAttribs);
	if (gfx->eglContext == EGL_NO_CONTEXT) {
		CEE_ERROR("Failed to create EGL context: 0x%X", eglGetError());
		return -1;
	}

	gfx->eglSurface = eglCreatePlatformWindowSurfaceEXT(gfx->eglDisplay,
										  gfx->eglConfig,
										  gfx->fb.gbmSurface,
										  NULL);

	if (gfx->eglSurface == EGL_NO_SURFACE) {
		CEE_ERROR("Failed to create EGL surface: 0x%X", eglGetError());
		return -1;
	}

	if (!eglMakeCurrent(gfx->eglDisplay, gfx->eglSurface, gfx->eglSurface, gfx->eglContext)) {
		CEE_ERROR("Failed to make context current: 0x%X", eglGetError());
		return -1;
	}

	int glVersion;
	char msg[4] = { '\0', '\0', '\0', '\0' };

#if BUILD_GLES
	if ((glVersion = gladLoaderLoadGLES2()) == 0) {
		CEE_ERROR("Failed to load GLES2");
		strncpy("ES ", msg, 3);
#elif BUILD_GL
	if ((glVersion = gladLoaderLoadGL()) == 0) {
		CEE_ERROR("Failed to load GL");
#else
	if (1) {
#endif
		return -1;
	}

	CEE_DEBUG("Loaded OpenGL %s%d.%d", msg, GLAD_VERSION_MAJOR(glVersion), GLAD_VERSION_MINOR(glVersion));
	gfx->versionString = (const char*)glGetString(GL_VERSION);

	glClearColor(.0f, .0f, .0f, .0f);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(gfx->eglDisplay, gfx->eglSurface);

	if (HALGbmPageFlip(&gfx->fb)) {
		CEE_ERROR("Failed flip gbm buffers");
		return -1;
	}

	result = drmModeSetCrtc(gfx->dpy.fd,
						 gfx->dpy.crtcId,
						 gfx->fb.gbmFboId,
						 0, 0,
						 &gfx->dpy.connectorId,
						 1,
						 gfx->dpy.connectorMode);
	if (result) {
		CEE_DEBUG("drmModeSetCrtc: %s", strerror(errno));
		result = drmIsMaster(gfx->dpy.fd);
		if (!result) {
			CEE_DEBUG("Attempting to set drm master...");
			result = drmSetMaster(gfx->dpy.fd);
			if (result < 0 || !drmIsMaster(gfx->dpy.fd)) {
				CEE_ERROR("... Fail");
				return -1;
			}
			CEE_DEBUG("... Success");

			result = drmModeSetCrtc(gfx->dpy.fd,
								 gfx->dpy.crtcId,
								 gfx->fb.gbmFboId,
								 0, 0,
								 &gfx->dpy.connectorId,
								 1,
								 gfx->dpy.connectorMode);
			if (result < 0) {
				CEE_DEBUG("drmModeSetCrtc: %s", strerror(errno));
				return result;
			}
			CEE_DEBUG("Crtc set");
		} else {
			CEE_ERROR("Unable to set crtc");
			return -1;
		}
	}

	if ((result = HALDisplayPageFlip(&gfx->dpy, gfx->fb.gbmFbo, gfx->fb.gbmFboId)))
		return result;

	HALReleaseBuffer(&gfx->fb);

	return 0;
}

int HALGfxDRMShutdown(struct HALGfx_DRM *gfx)
{
	if (gfx == NULL) {
		return -1;
	}

	eglMakeCurrent(gfx->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(gfx->eglDisplay, gfx->eglSurface);
	eglDestroyContext(gfx->eglDisplay, gfx->eglContext);

	return 0;
}

void HALGfxDRMDestroy(struct HALGfx_DRM *gfx)
{
	free(gfx);
}

const char *HALGfxDRMGetVersionString(const struct HALGfx_DRM *gfx)
{
	return gfx->versionString;
}

int HALGfxDRMGetWidth(const struct HALGfx_DRM *gfx)
{
	return gfx->width;
}

int HALGfxDRMGetHeight(const struct HALGfx_DRM *gfx)
{
	return gfx->height;
}

int HALGfxDRMPageFlip(struct HALGfx_DRM *gfx)
{
	static int fc = 1;
	int ret;
	if (eglSwapBuffers(gfx->eglDisplay, gfx->eglSurface) != GL_TRUE) {
		CEE_ERROR("eglSwapBuffers failed: 0x%X", eglGetError());
		return -1;
	}
	
	if ((ret = HALGbmPageFlip(&gfx->fb)))
		return ret;
	
	if ((ret = HALDisplayPageFlip(&gfx->dpy, gfx->fb.gbmFbo, gfx->fb.gbmFboId))) {
		CEE_DEBUG("Drm page flipped failed, releasing buffer");

		drmModeRmFB(gfx->dpy.fd, gfx->fb.gbmFboId);
		HALReleaseBuffer(&gfx->fb);

		return ret;
	}
	
	HALReleaseBuffer(&gfx->fb);
	
	fc++;

	return 0;
}

