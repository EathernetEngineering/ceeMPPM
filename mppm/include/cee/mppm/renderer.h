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

#ifndef CEE_RENDERER_H_
#define CEE_RENDERER_H_

#include <cee/hal/gfx.h>

#include <memory>

#include <glm/glm.hpp>

namespace cee {
struct Vertex {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec2 uv;
};

class Renderer {
public:
	Renderer(){}
	virtual ~Renderer(){}

	virtual void SwapBuffers() = 0;

	virtual void Clear() = 0;
	virtual void ClearColor(const glm::vec4 &color) = 0;

	virtual void SetColor(const glm::vec4 &color) = 0;

	virtual void DrawQuad(const glm::vec3 &position, const glm::vec3 &scale) = 0;

	virtual void Flush() = 0;

	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;

	inline int GetWidth() const { return HALGfxGetWidth(m_HALGfx); }
	inline int GetHeight() const { return HALGfxGetHeight(m_HALGfx); }

	inline std::string GetVersionString() { return m_VersionString; }

	static std::unique_ptr<Renderer> Create();

protected:
	HALGfx *m_HALGfx;
	std::string m_VersionString;
};
}

#endif
