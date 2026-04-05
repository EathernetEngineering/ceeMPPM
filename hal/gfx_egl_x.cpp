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
#include <config.h>

#include <glad/egl.h>
#include <glad/gles2.h>

#include <X11/Xlib.h>

#include <signal.h>

namespace cee {
namespace hal {
	X11GraphicsContext::X11GraphicsContext()
	 : m_Display(nullptr), m_Window(0), m_Screen(0), m_WmDeleteAtom(0),
	   m_EglDisplay(EGL_NO_DISPLAY), m_EglSurface(EGL_NO_SURFACE), m_EglContext(EGL_NO_CONTEXT)
	{ }

	X11GraphicsContext::~X11GraphicsContext()
	{
		if (m_Display) {
			Shutdown();
		}
	}

	void X11GraphicsContext::Init()
	{
		m_Display = XOpenDisplay(NULL);
		if (m_Display == 0) {
			CEE_ERROR("Failed to open X display");
			throw std::runtime_error("Failed to open X display");
		}

		m_Screen = DefaultScreen(m_Display);

		if (!gladLoaderLoadEGL(EGL_NO_DISPLAY)) {
			CEE_ERROR("EGL loader error");
			throw std::runtime_error("Failed to load egl function pointers");
		}

		Window root = RootWindow(m_Display, m_Screen);

		XSetWindowAttributes setWinAttribs;
		setWinAttribs.event_mask = ExposureMask | StructureNotifyMask;
		m_Window = XCreateWindow(m_Display, root,
				0, 0,
				1280, 720,
				0, CopyFromParent, InputOutput,
				CopyFromParent, CWEventMask, &setWinAttribs);

		XMapWindow(m_Display, m_Window);
		XStoreName(m_Display, m_Window, "ceeHAL X11");

		m_EglDisplay = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(m_Display));
		if (m_EglDisplay == EGL_NO_DISPLAY) {
			CEE_ERROR("Unable to get EGLDisplay from X");
			XDestroyWindow(m_Display, m_Window);
			throw std::runtime_error("Failed to get EGL display from X");
		}
		m_WmDeleteAtom = XInternAtom(m_Display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(m_Display, m_Window, &m_WmDeleteAtom, 1);

		EGLint major, minor;
		if (!eglInitialize(m_EglDisplay, &major, &minor)) {
			CEE_ERROR("Failed to initialize EGL");
			XDestroyWindow(m_Display, m_Window);
			throw std::runtime_error("Failed to initialize EGL");
		}
		CEE_DEBUG("Loaded EGL {}.{}", major, minor);

		if (!gladLoaderLoadEGL(m_EglDisplay)) {
			CEE_ERROR("Failed to load EGL");
			XDestroyWindow(m_Display, m_Window);
			throw std::runtime_error("Failed to load egl function pointers with display");
		}

		eglBindAPI(EGL_OPENGL_ES2_BIT);

		EGLint surfaceAttributes[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_NONE
		};

		EGLConfig config;
		EGLint nConfigs;
		eglChooseConfig(m_EglDisplay, surfaceAttributes, &config, 1, &nConfigs);

		m_EglSurface = eglCreateWindowSurface(m_EglDisplay,
				config,
				reinterpret_cast<EGLNativeWindowType>(m_Window),
				NULL);
		if (m_EglSurface == EGL_NO_SURFACE) {
			CEE_ERROR("Failed to create EGL surface");
			XDestroyWindow(m_Display, m_Window);
			throw std::runtime_error("Failed to create EGL surface");
		}

		EGLint contextAttribs[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 2,
			EGL_CONTEXT_MINOR_VERSION_KHR, 0,
			EGL_NONE
		};
		m_EglContext = eglCreateContext(m_EglDisplay,
										config,
										EGL_NO_CONTEXT,
										contextAttribs);
		if (m_EglContext == EGL_NO_CONTEXT) {
			CEE_ERROR("Failed to create EGL context");
			XDestroyWindow(m_Display, m_Window);
			eglDestroySurface(m_EglDisplay, m_EglSurface);
			throw std::runtime_error("Failed to create EGL context");
		}

		if (!eglMakeCurrent(m_EglDisplay, m_EglSurface, m_EglSurface, m_EglContext)) {
			CEE_ERROR("Failed to make EGL context current");
			XDestroyWindow(m_Display, m_Window);
			eglDestroyContext(m_EglDisplay, m_EglContext);
			eglDestroySurface(m_EglDisplay, m_EglSurface);
			throw std::runtime_error("Failed to make EGL context current");
		}	

		int glVersion;
		if ((glVersion = gladLoaderLoadGLES2()) == 0) {
			CEE_ERROR("Failed to load OpenGL ES");
		}

		CEE_DEBUG("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(glVersion), GLAD_VERSION_MINOR(glVersion));
		CEE_DEBUG("\tVersion: {}", this->GetVersionString());
		CEE_DEBUG("\tShading Language Version: {}", this->GetShadingVersionString());
		CEE_DEBUG("\tRenderer: {}", (const char *)glGetString(GL_RENDERER));

		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(m_EglDisplay, m_EglSurface);
	}

	void X11GraphicsContext::Shutdown() {
		eglDestroySurface(m_EglDisplay, m_EglSurface);
		m_EglSurface = EGL_NO_SURFACE;
		eglDestroyContext(m_EglDisplay, m_EglContext);
		m_EglContext = EGL_NO_CONTEXT;
		eglTerminate(m_EglDisplay);
		m_EglDisplay = EGL_NO_DISPLAY;
		XDestroyWindow(m_Display, m_Window);
		m_Window = 0;
		XCloseDisplay(m_Display);
		m_Display = nullptr;
	}

	const char* X11GraphicsContext::GetVersionString() const
	{
		return reinterpret_cast<const char *>(glGetString(GL_VERSION));
	}

	const char* X11GraphicsContext::GetShadingVersionString() const
	{
		return reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	void X11GraphicsContext::SwapBuffers()
	{
		if (!eglSwapBuffers(m_EglDisplay, m_EglSurface)) {
			CEE_WARN("eglSwapBuffers failed: 0x{:X}", glGetError());
			throw std::runtime_error("eglSwapBuffers failed");
		}

		XEvent event;
		while (XPending(m_Display)) {
			XNextEvent(m_Display, &event);

			switch (event.type) {
				case ConfigureNotify: {
					m_Width = event.xconfigure.width;
					m_Height = event.xconfigure.height;
				} break;

				case ClientMessage: {
					if ((Atom)event.xclient.data.l[0] == m_WmDeleteAtom) {
						raise(SIGTRAP);
						return;
					}
				} break;

				default: {
					// Discard other events
				}
			}
		}
	}
}
}

