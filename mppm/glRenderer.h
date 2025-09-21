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

#ifndef CEE_GL_RENDERER_H_
#define CEE_GL_RENDERER_H_

#include <cee/mppm/renderer.h>

#include <glm/glm.hpp>
#include <glad/gl.h>

namespace cee {
class OpenGLRenderer : public Renderer {
public:
	OpenGLRenderer();
	virtual ~OpenGLRenderer();

	virtual void SwapBuffers() override;

	virtual void Clear() override;
	virtual void ClearColor(const glm::vec4 &color) override;

	virtual void SetColor(const glm::vec4 &color) override;

	virtual void DrawQuad(const glm::vec3 &position, const glm::vec3 &scale) override;

	virtual void Flush() override;

	virtual void StartFrame() override;
	virtual void EndFrame() override;

protected:
	glm::vec4 m_ObjColor;
	glm::vec4 m_ClearColor;

	struct {
		float *vertices;
		uint16_t vertexCount;
		uint16_t *indices;
		uint16_t indexCount;

		GLuint vbo;
		GLuint ebo;
	} m_Triangles, m_Quads;
};
}

#endif

