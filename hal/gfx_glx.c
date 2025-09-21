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
#include <gfx_glx.h>
#include <log.h>
#include <util.h>

#include <glad/gl.h>
#include <glad/glx.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

struct HALGfx_glx {
	Display *xDisplay;
	int xScreen;
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
	xcb_atom_t wmProtocolsAtom;
	xcb_atom_t wmDeleteWindowAtom;
	GLXContext glxContext;
	GLXWindow glxWindow;
	GLXDrawable drawable;
	const char *versionString;

	int width, height;
};

struct HALGfx_glx *HALGfxGlXCreate(void)
{
	return calloc(1, sizeof(struct HALGfx_glx));
}

int HALGfxGlXInit(struct HALGfx_glx *gfx)
{
	if (gfx == NULL) {
		return -1;
	}

	gfx->xDisplay = XOpenDisplay(NULL);
	if (!gfx->xDisplay) {
		CEE_ERROR("Failed to open X display");
		return -1;
	}

	gfx->xScreen = DefaultScreen(gfx->xDisplay);

	gfx->connection = XGetXCBConnection(gfx->xDisplay);
	if (gfx->connection == NULL) {
		XCloseDisplay(gfx->xDisplay);
		CEE_ERROR("Failed to get xcb connection from X");;
		return -1;
	}

	XSetEventQueueOwner(gfx->xDisplay, XCBOwnsEventQueue);

	const xcb_setup_t *setup = xcb_get_setup(gfx->connection);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	gfx->screen = iter.data;

	return 0;
}

int HALGfxGlXShutdown(struct HALGfx_glx *gfx)
{
	if (gfx == NULL) {
		return -1;
	}
	if (gfx->glxWindow)
		glXDestroyWindow(gfx->xDisplay, gfx->glxWindow);
	if (gfx->window)
		xcb_destroy_window(gfx->connection, gfx->window);
	if (gfx->glxContext)
		glXDestroyContext(gfx->xDisplay, gfx->glxContext);
	if (gfx->xDisplay)
		XCloseDisplay(gfx->xDisplay);

	return 0;
}

void HALGfxGlXDestroy(struct HALGfx_glx *gfx)
{
	free(gfx);
}

const char *HALGfxGlXGetVersionString(const struct HALGfx_glx *gfx)
{
	return gfx->versionString;
}

int HALGfxGlXGetWidth(const struct HALGfx_glx *gfx)
{
	return gfx->width;
}

int HALGfxGlXGetHeight(const struct HALGfx_glx *gfx)
{
	return gfx->height;
}

int HALGfxGlXPageFlip(struct HALGfx_glx *gfx)
{
	xcb_generic_event_t *event;
	while ((event = xcb_poll_for_event(gfx->connection))) {
		if (!event) {
			return -1;
		}
		switch (event->response_type & ~0x80) {
			case XCB_EXPOSE:
				{
					;
				}
				break;

			case XCB_CONFIGURE_NOTIFY:
				{
					xcb_configure_notify_event_t *configure = (void*)event;
					gfx->width = configure->width;
					gfx->height = configure->height;
				}
				break;

			case XCB_CLIENT_MESSAGE:
				{
					xcb_client_message_event_t *clientMessage = (void*)event;
					if (clientMessage->type == gfx->wmProtocolsAtom &&
						clientMessage->data.data32[0] == gfx->wmDeleteWindowAtom) {
						raise(SIGHUP);
					}
				}
				break;

			default:
				break;
		}
		free(event);
	}
					glXSwapBuffers(gfx->xDisplay, gfx->drawable);

	return 0;
}

int HALGfxGlXCreateWindow(struct HALGfx_glx *gfx, int width, int height, const char *title)
{
	if (gfx == NULL) {
		return -1;
	}
	int visualID = 0;

	int glx_version = gladLoaderLoadGLX(gfx->xDisplay, gfx->xScreen);
	if (!glx_version) {
		CEE_ERROR("Failed to load GLX.");
		return -1;
	}
	CEE_DEBUG("Loaded GLX %d.%d\n", GLAD_VERSION_MAJOR(glx_version), GLAD_VERSION_MINOR(glx_version));

	GLXFBConfig *fbConfigs = NULL;
	int nfbConfigs = 0;
	fbConfigs = glXGetFBConfigs(gfx->xDisplay, gfx->xScreen, &nfbConfigs);
	if (!fbConfigs || nfbConfigs == 0) {
		CEE_ERROR("No GLX configs available");
		return -1;
	}

	GLXFBConfig fbConfig = fbConfigs[0];
	glXGetFBConfigAttrib(gfx->xDisplay, fbConfig, GLX_VISUAL_ID, &visualID);

	GLXContext context;
	context = glXCreateNewContext(gfx->xDisplay, fbConfig, GLX_RGBA_TYPE, 0, True);
	if (!context) {
		CEE_ERROR("Failed to create GLX context");
		return -1;
	}

	xcb_colormap_t colormap = xcb_generate_id(gfx->connection);
	gfx->window = xcb_generate_id(gfx->connection);

	xcb_create_colormap(gfx->connection,
					 XCB_COLORMAP_ALLOC_NONE,
					 colormap,
					 gfx->screen->root,
					 visualID);

	uint32_t eventmask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
	uint32_t valuelist[] = { eventmask, colormap, 0 };
	uint32_t valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

	xcb_create_window(gfx->connection,
				   XCB_COPY_FROM_PARENT,
				   gfx->window,
				   gfx->screen->root,
				   0, 0,
				   640, 480,
				   0,
				   XCB_WINDOW_CLASS_INPUT_OUTPUT,
				   visualID,
				   valuemask, valuelist);
	xcb_map_window(gfx->connection, gfx->window);

	GLXWindow glxWindow = glXCreateWindow(gfx->xDisplay, fbConfig, gfx->window, 0);

	if (!gfx->window) {
		xcb_destroy_window(gfx->connection, gfx->window);
		glXDestroyContext(gfx->xDisplay, context);
		CEE_ERROR("Failed to create window");

		return -1;
	}

	gfx->drawable = glxWindow;

	if (!glXMakeContextCurrent(gfx->xDisplay, gfx->drawable, gfx->drawable, context)) {
		xcb_destroy_window(gfx->connection, gfx->window);
		glXDestroyContext(gfx->xDisplay, context);
		CEE_ERROR("Failed to make GLX context current");

		return -1;
	}

	int glVersion;
	char msg[4] = { '\0', '\0', '\0', '\0'};

#if BUILD_GLES
	if ((gl_version = gladLoaderLoadGLES2()) == 0) {
		CEE_ERROR("Failed to load GLES2");
		strncpy("ES ", msg, 3);
#elif BUILD_GL
	if ((gl_version = gladLoaderLoadGL()) == 0) {
		CEE_ERROR("Failed to load GL");
#else
	if (1) {
#endif
		return -1;
	}

	CEE_DEBUG("Loaded OpenGL %s%d.%d\n", msg, GLAD_VERSION_MAJOR(glVersion), GLAD_VERSION_MINOR(glVersion));
	gfx->versionString = (const char *)glGetString(GL_VERSION);

	const char *wmProtocolsAtomName = "WM_PROTOCOLS";
	xcb_intern_atom_cookie_t windowCloseCookie = xcb_intern_atom(
															  gfx->connection,
															  0,
															  strlen(wmProtocolsAtomName),
															  wmProtocolsAtomName);
	xcb_intern_atom_reply_t *windowCloseReply = xcb_intern_atom_reply(
																   gfx->connection,
																   windowCloseCookie,
																   NULL);
	if (!windowCloseReply) {
		CEE_ERROR("Failed to get WM_PROTOCOLS reply");
		return -1;
	}
	xcb_atom_t wmProtocolsAtom = windowCloseReply->atom;
	free(windowCloseReply);

	const char *wmDeleteWindowAtomName = "WM_DELETE_WINDOW";
	windowCloseCookie = xcb_intern_atom(
									 gfx->connection,
									 0,
									 16,
									 wmDeleteWindowAtomName);
	windowCloseReply = xcb_intern_atom_reply(
										  gfx->connection, 
										  windowCloseCookie,
										  NULL);
	if (!windowCloseReply) {
		CEE_ERROR("Failed to get WM_DELETE_WINDOW reply");
		return -1;
	}
	xcb_atom_t wmDeleteWindowAtom = windowCloseReply->atom;
	free(windowCloseReply);


	xcb_change_property(
					 gfx->connection,
					 XCB_PROP_MODE_REPLACE,
					 gfx->window,
					 wmProtocolsAtom,
					 XCB_ATOM_ATOM,
					 32,
					 1,
					 &wmDeleteWindowAtom);

	gfx->wmProtocolsAtom = wmProtocolsAtom;
	gfx->wmDeleteWindowAtom = wmDeleteWindowAtom;

	xcb_flush(gfx->connection);

	xcb_get_geometry_cookie_t geometryCookie;
	geometryCookie = xcb_get_geometry(gfx->connection, gfx->window);
	xcb_get_geometry_reply_t *geometryReply;
	xcb_generic_error_t *error;
	geometryReply = xcb_get_geometry_reply(gfx->connection, geometryCookie, &error);
	if (error) {
		CEE_WARN("Failed to get window size");
		free(error);
	} else {
		gfx->width = geometryReply->width;
		gfx->height = geometryReply->height;
		free(geometryReply);
	}

	return 0;	
}


