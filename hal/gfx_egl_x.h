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

#ifndef CEE_HAL_GFX_EGL_X_H_
#define CEE_HAL_GFX_EGL_X_H_

#include <cee/hal/gfx.h>
#include <X11/Xlib.h>
#include <X11/X.h>

#include <glad/egl.h>

namespace cee {
namespace hal {
	class X11GraphicsContext : public GraphicsContext {
		protected:
			X11GraphicsContext();

		public:
			virtual ~X11GraphicsContext();

			virtual void Init() override;
			virtual void Shutdown() override;

			virtual const char* GetVersionString() const override;
			virtual const char* GetShadingVersionString() const override;
			virtual void SwapBuffers() override;

		protected:
			Display *m_Display;
			Window m_Window;
			int m_Screen;
			Atom m_WmDeleteAtom;

			EGLDisplay m_EglDisplay;
			EGLSurface m_EglSurface;
			EGLContext m_EglContext;

		public:
			friend std::unique_ptr<GraphicsContext> GraphicsContext::Create();
	};
}
}

#endif

