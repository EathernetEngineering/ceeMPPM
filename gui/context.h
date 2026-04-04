/*
 * CeeHealth
 * Copyright (C) 2026 Chloe Eather
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

#ifndef CEE_GUI_CONTEXT_H_
#define CEE_GUI_CONTEXT_H_

#include <cee/gui/object.h>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <stack>
#include <unordered_map>

namespace cee {
namespace gui {
	class Context {
	public:
		enum class Shader {
			Flat
		};

		struct Vertex {
			glm::vec4 position;
			Color color;
		};
	public:
		Context();
		~Context();
		void SetViewport(const glm::vec2 &viewport);

		void PushClip(const glm::vec4 &clip);
		void PopClip();
		void PushTransform(const glm::mat4 &transform);
		void PopTransform();

		void DrawRect(const Rect &rect, const Color &color);
		void Flush();

		void UseShader(Shader shader);
		void SetUniform(const std::string &name, const glm::mat4 &value);

		const glm::mat4& GetProjection() const { return m_Projection; }

	private:
		GLint GetUniformLocation(const std::string& name);

	private:
		uint32_t m_FlatShader;
		uint32_t m_VAO, m_VBO, m_EBO;
		uint32_t m_VertexCount, m_IndexCount;
		glm::vec2 m_Viewport;
		glm::mat4 m_Projection;
		std::unordered_map<std::string, GLint> m_UniformLocations;
		std::array<Vertex, 1024> m_VertexData;
		std::array<uint16_t, 1024> m_IndexData;
		std::stack<glm::mat4> m_Transform;
		std::stack<glm::vec4> m_Clip;
	};
}
}

#endif

