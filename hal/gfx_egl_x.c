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
#include <gfx_egl_x.h>
#include <log.h>
#include <util.h>
#include <config.h>

#include <glad/egl.h>
#if BUILD_GLES
#include <glad/gles2.h>
#elif BUILD_GL
#include <glad/gl.h>
#endif
#include <X11/Xlib.h>
#include <X11/X.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

struct HALGfx_egl_x {
	Display *xDisplay;
	Window xWindow;
	int xScreen;
	Atom xWmDeleteAtom;

	EGLDisplay eglDisplay;
	EGLSurface eglSurface;
	EGLContext eglContext;
	const char *versionString;
	int width;
	int height;
};

struct HALGfx_egl_x *HALGfxEglXCreate(void)
{
	return calloc(1, sizeof(struct HALGfx_egl_x));
}

int HALGfxEglXInit(struct HALGfx_egl_x *gfx)
{
	gfx->xDisplay = XOpenDisplay(NULL);
	if (!gfx->xDisplay) {
		CEE_ERROR("Failed to open X display");
		return -1;
	}

	gfx->xScreen = DefaultScreen(gfx->xDisplay);

	if (!gladLoaderLoadEGL(EGL_NO_DISPLAY)) {
		CEE_ERROR("Failed to load initial egl functions");
		return -1;
	}

	return 0;
}

int HALGfxEglXShutdown(struct HALGfx_egl_x *gfx)
{
	eglDestroySurface(gfx->eglDisplay, gfx->eglSurface);
	eglDestroyContext(gfx->eglDisplay, gfx->eglContext);
	eglTerminate(gfx->eglDisplay);
	XDestroyWindow(gfx->xDisplay, gfx->xWindow);
	XCloseDisplay(gfx->xDisplay);

	return 0;
}

void HALGfxEglXDestroy(struct HALGfx_egl_x *gfx)
{
	free(gfx);
}

const char *HALGfxEglXGetVersionString(const struct HALGfx_egl_x *gfx)
{
	return gfx->versionString;
}

int HALGfxEglXGetWidth(const struct HALGfx_egl_x *gfx)
{
	return gfx->width;
}

int HALGfxEglXGetHeight(const struct HALGfx_egl_x *gfx)
{
	return gfx->height;
}

int HALGfxEglXPageFlip(struct HALGfx_egl_x *gfx)
{
	if (!eglSwapBuffers(gfx->eglDisplay, gfx->eglSurface)) {
		CEE_WARN("eglSwapBuffers failed: 0x%X", glGetError());
		return -1;
	}

	XEvent event;
	while (XPending(gfx->xDisplay)) {
		XNextEvent(gfx->xDisplay, &event);

		switch (event.type) {
			case ConfigureNotify: {
				gfx->width = event.xconfigure.width;
				gfx->height = event.xconfigure.height;
			} break;

			case ClientMessage: {
				if ((Atom)event.xclient.data.l[0] == gfx->xWmDeleteAtom) {
					raise(SIGTRAP);
					return 0;
				}
			} break;

			default: {
				// Discard other events
			}
		}
	}

	return 0;
}

int HALGfxEglXCreateWindow(struct HALGfx_egl_x *gfx, int width, int height, const char *title)
{
	Window root = RootWindow(gfx->xDisplay, gfx->xScreen);

	XSetWindowAttributes setWinAttribs;
	setWinAttribs.event_mask = ExposureMask | StructureNotifyMask;
	gfx->xWindow = XCreateWindow(gfx->xDisplay, root,
							  0, 0,
							  1280, 720,
							  0,
							  CopyFromParent, InputOutput,
							  CopyFromParent, CWEventMask, &setWinAttribs);

	XMapWindow(gfx->xDisplay, gfx->xWindow);
	XStoreName(gfx->xDisplay, gfx->xWindow, "ceeHAL EGL X11");

	gfx->eglDisplay = eglGetDisplay((EGLNativeDisplayType)gfx->xDisplay);
	if (gfx->eglDisplay == EGL_NO_DISPLAY) {
		CEE_ERROR("Unable to get EGLDisplay from X display");
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		return -1;
	}
	gfx->xWmDeleteAtom = XInternAtom(gfx->xDisplay, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(gfx->xDisplay, gfx->xWindow, &gfx->xWmDeleteAtom, 1);

	EGLint major, minor;
	if (!eglInitialize(gfx->eglDisplay, &major, &minor)) {
		CEE_ERROR("Failed to load EGL");
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		return -1;
	}
	CEE_DEBUG("Loaded EGL %i.%i", major, minor);

	if (!gladLoaderLoadEGL(gfx->eglDisplay)) {
		CEE_ERROR("Failed to load egl functions with display");
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		return -1;
	}

	eglBindAPI(EGL_OPENGL_ES2_BIT);

	EGLint surfaceAttributes[] = {
#if BUILD_GLES
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#elif BUILD_GL
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_NONE
	};

	EGLConfig config;
	EGLint nConfigs;
	eglChooseConfig(gfx->eglDisplay, surfaceAttributes, &config, 1, &nConfigs);

	gfx->eglSurface = eglCreateWindowSurface(gfx->eglDisplay,
										  config,
										  (EGLNativeWindowType)gfx->xWindow,
										  NULL);
	if (gfx->eglSurface == EGL_NO_SURFACE) {
		CEE_ERROR("Failed to create egl surface");
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		return -1;
	}

	EGLint contextAttribs[] = {
#if BUILD_GLES
		EGL_CONTEXT_MAJOR_VERSION_KHR, 2,
		EGL_CONTEXT_MINOR_VERSION_KHR, 0,
#elif BUILD_GL
#endif
		EGL_NONE
	};
	gfx->eglContext = eglCreateContext(gfx->eglDisplay,
									config,
									EGL_NO_CONTEXT,
									contextAttribs);
	if (gfx->eglContext == EGL_NO_CONTEXT) {
		CEE_ERROR("Failed to create egl context");
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		eglDestroySurface(gfx->eglDisplay, gfx->eglSurface);
		return -1;
	}

	if (!eglMakeCurrent(gfx->eglDisplay, gfx->eglSurface, gfx->eglSurface, gfx->eglContext)) {
		CEE_ERROR("Failed to make context current");
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		eglDestroyContext(gfx->eglDisplay, gfx->eglContext);
		eglDestroySurface(gfx->eglDisplay, gfx->eglSurface);
		return -1;
	}	

	int glVersion;
	char msg[4] = { '\0', '\0', '\0', '\0' };
#if BUILD_GLES
	if ((glVersion = gladLoaderLoadGLES2()) == 0) {
		CEE_ERROR("Failed to load OpenGL ES");
		strncpy("ES ", msg, 3);
#elif BUILD_GL
	if ((glVersion = gladLoaderLoadGL()) == 0) {
		CEE_ERROR("Failed to load OpenGL");
#else
	if (1) {
#endif
		XDestroyWindow(gfx->xDisplay, gfx->xWindow);
		eglDestroyContext(gfx->eglDisplay, gfx->eglContext);
		eglDestroySurface(gfx->eglDisplay, gfx->eglSurface);
		return -1;
	}

	CEE_DEBUG("Loaded OpenGL %s%d.%d", msg, GLAD_VERSION_MAJOR(glVersion), GLAD_VERSION_MINOR(glVersion));
	gfx->versionString = (const char *)glGetString(GL_VERSION);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(gfx->eglDisplay, gfx->eglSurface);

	return 0;	
}


