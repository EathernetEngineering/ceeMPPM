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

#ifndef CEE_HAL_GFX_H_
#define CEE_HAL_GFX_H_

#include <memory>

namespace cee {
namespace hal {
class GraphicsContext {
	public:
		GraphicsContext()
			: m_Width(0u), m_Height(0u)
		{}
		virtual ~GraphicsContext(){}

	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual const char* GetVersionString() const = 0;
		virtual const char* GetShadingVersionString() const = 0;
		virtual int GetWidth() const { return m_Width; }
		virtual int GetHeight() const { return m_Height; }
		virtual void SwapBuffers() = 0;

	public:
		static std::unique_ptr<GraphicsContext> Create();

	protected:
		uint32_t m_Width, m_Height;
};
}
}

#endif

