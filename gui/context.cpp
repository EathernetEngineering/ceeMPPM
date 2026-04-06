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

#include <context.h>
#include <cee/gui/object.h>
#include <log.h>
#include <shaders.h>

#include <cee/profiler/profiler.h>

#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

#include <stdexcept>
#include <format>

namespace cee {
namespace gui {
	static int BuildShader(std::string_view vertSrc, std::string_view fragSrc, GLuint *outProg);

	Context::Context()
	 : m_VertexCount(0), m_IndexCount(0) {
		glGenBuffers(1, &m_VBO);
		if (m_VBO == 0) {
			throw std::runtime_error("Failed to create OpenGL vertex buffer object");
		}
		glGenBuffers(1, &m_EBO);
		if (m_EBO == 0) {
			throw std::runtime_error("Failed to create OpenGL index buffer object");
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);

		glBufferData(GL_ARRAY_BUFFER, 1024, nullptr, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1024, nullptr, GL_DYNAMIC_DRAW);

		if (BuildShader(vertexShaderSourceES2, fragmentShaderSourceES2, &m_QuadFlatShader) != 0) {
			CEE_WARN("Failed to compile GLSL ES 100 shader, Trying GLSL ES 320");
			if (BuildShader(vertexShaderSourceES3, fragmentShaderSourceES3, &m_QuadFlatShader) != 0)
				throw std::runtime_error("Could not build shaders!");
		}
		CEE_DEBUG("Flat shader compiled successfully");

		m_Projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);
	}

	Context::~Context() {
		glUseProgram(GL_NONE);
		glDeleteProgram(m_QuadFlatShader);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}

	void Context::SetViewport(const glm::vec2 &viewport) {
		if (viewport.x == m_Viewport.x && viewport.y == m_Viewport.y)
			return;
		m_Viewport = viewport;
		m_Projection = glm::ortho(0.f, m_Viewport.x, m_Viewport.y, 0.f);
	}

	void Context::PushClip(const glm::vec4 &clip) {
		Flush();
		// TODO: Set scissor rect to clip
		m_Clip.push(clip);
	}

	void Context::PopClip() {
		Flush();
		m_Clip.pop();
	}

	void Context::PushTransform(const glm::mat4 &transform) {
		Flush();
		m_Transform.push(transform);
		try {
			SetUniform("uTrans", m_Transform.top());
		} catch (const std::exception& e) {
			// This can fail if the shader doesn't have a uTrans uniform.
			// Thats fine.
			CEE_WARN("Failed to set transform uniform: {}", e.what());
		}
	}

	void Context::PopTransform() {
		Flush();
		m_Transform.pop();
	}

	void Context::DrawRect(const Rect& rect, const Color& color) {
		if (m_VertexCount + 4 > 1024 || m_IndexCount + 6 > 1024) {
			Flush();
		}
		m_VertexData[m_VertexCount++] = { { rect.x,          rect.y,          0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { rect.x + rect.w, rect.y,          0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { rect.x + rect.w, rect.y + rect.h, 0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { rect.x,          rect.y + rect.h, 0.0f, 1.0f }, color };
		m_IndexData[m_IndexCount++] = m_VertexCount - 4;
		m_IndexData[m_IndexCount++] = m_VertexCount - 3;
		m_IndexData[m_IndexCount++] = m_VertexCount - 2;
		m_IndexData[m_IndexCount++] = m_VertexCount - 2;
		m_IndexData[m_IndexCount++] = m_VertexCount - 1;
		m_IndexData[m_IndexCount++] = m_VertexCount - 4;
	}

	void Context::DrawTriangle(const Point &a, const Point &b, const Point &c, const Color &color)
	{
		if (m_VertexCount + 3 > 1024 || m_IndexCount + 3 > 1024) {
			Flush();
		}
		m_VertexData[m_VertexCount++] = { { a.x, a.y, 0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { b.x, b.y, 0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { c.x, c.y, 0.0f, 1.0f }, color };
		m_IndexData[m_IndexCount++] = m_VertexCount - 3;
		m_IndexData[m_IndexCount++] = m_VertexCount - 2;
		m_IndexData[m_IndexCount++] = m_VertexCount - 1;
	}

	void Context::DrawLine(const Point &p1, const Point &p2, float width, const Color &color)
	{
		if (m_VertexCount + 4 > 1024 || m_IndexCount + 6 > 1024) {
			Flush();
		}
		glm::vec2 dir = glm::normalize(glm::vec2(p2.x, p2.y) - glm::vec2(p1.x, p1.y));
		glm::vec2 normal = glm::vec2(-dir.y, dir.x) * width * 0.5f;
		m_VertexData[m_VertexCount++] = { { glm::vec2(p1.x, p1.y) + normal, 0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { glm::vec2(p2.x, p2.y) + normal, 0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { glm::vec2(p2.x, p2.y) - normal, 0.0f, 1.0f }, color };
		m_VertexData[m_VertexCount++] = { { glm::vec2(p1.x, p1.y) - normal, 0.0f, 1.0f }, color };
		m_IndexData[m_IndexCount++] = m_VertexCount - 4;
		m_IndexData[m_IndexCount++] = m_VertexCount - 3;
		m_IndexData[m_IndexCount++] = m_VertexCount - 2;
		m_IndexData[m_IndexCount++] = m_VertexCount - 2;
		m_IndexData[m_IndexCount++] = m_VertexCount - 1;
		m_IndexData[m_IndexCount++] = m_VertexCount - 4;
	}

	void Context::Flush() {
		PROFILE_SCOPE("GUI flush buffers");
		if (m_IndexCount == 0)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexCount * sizeof(Vertex), m_VertexData.data());
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_IndexCount * sizeof(uint16_t), m_IndexData.data());
		glUseProgram(m_QuadFlatShader);
		glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_SHORT, nullptr);
		m_VertexCount = 0;
		m_IndexCount = 0;
	}

	void Context::UseShader(Shader shader) {
		switch (shader) {
			case Shader::Flat:
				glUseProgram(m_QuadFlatShader);
				break;
			default:
				throw std::logic_error("Unknown shader");
		}
	}

	void Context::SetUniform(const std::string& name, const glm::mat4& value) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	GLint Context::GetUniformLocation(const std::string& name) {
		PROFILE_SCOPE("GUI fetch uniform location");
		GLint location;
		auto it = m_UniformLocations.find(name);
		if (it == m_UniformLocations.end()) {
			PROFILE_SCOPE("Uniform cache miss");
			location = glGetUniformLocation(m_QuadFlatShader, name.c_str());
			if (location == -1) {
				throw std::runtime_error(std::format("Uniform '{}' not found in shader", name));
			}
			m_UniformLocations[name] = location;
		} else {
			location = it->second;
		}
		return location;
	}

	static int BuildShader(std::string_view vertSrc, std::string_view fragSrc, GLuint *outProg) {
		PROFILE_FUNCTION();
		GLuint vertShader, fragShader, prog;
		GLint success;
		const char *const vertStrs[] = { vertSrc.data() };
		const char *const fragStrs[] = { fragSrc.data() };
		if (outProg == nullptr)
			return -1;

		vertShader = glCreateShader(GL_VERTEX_SHADER);
		if (vertShader == 0)
			return -1;
		glShaderSource(vertShader, 1, vertStrs, NULL);
		glCompileShader(vertShader);
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint logLength;
			char *log;
			glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
			log = new char[logLength];
			if (log == nullptr) {
				glDeleteShader(vertShader);
				return -1;
			}
			glGetShaderInfoLog(vertShader, logLength, NULL, log);
			CEE_WARN("Failed to compile vertex shader:\n{}", log);
			delete[] log;
			glDeleteShader(vertShader);
			return -1;
		}
		fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		if (fragShader == 0)
			return -1;
		glShaderSource(fragShader, 1, fragStrs, NULL);
		glCompileShader(fragShader);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char *log;
			GLint logLength;
			glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
			log = new char[logLength];
			if (log == nullptr) {
				glDeleteShader(vertShader);
				glDeleteShader(fragShader);
				return -1;
			}
			glGetShaderInfoLog(fragShader, logLength, NULL, log);
			CEE_WARN("Failed to compile fragment shader:\n{}", log);
			delete[] log;
			glDeleteShader(vertShader);
			glDeleteShader(fragShader);
			return -1;
		}
		prog = glCreateProgram();
		if (prog == 0)
			return -1;
		glAttachShader(prog, vertShader);
		glAttachShader(prog, fragShader);
		glLinkProgram(prog);
		glGetProgramiv(prog, GL_LINK_STATUS, &success);
		if (!success) {
			char *log;
			GLint logLength;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
			log = new char[logLength];
			if (log == nullptr) {
				glDeleteShader(vertShader);
				glDeleteShader(fragShader);
				glDeleteProgram(prog);
				return -1;
			}
			glGetProgramInfoLog(prog, logLength, NULL, log);
			CEE_WARN("Failed to link shader:\n{}", log);
			delete[] log;
			glDeleteShader(vertShader);
			glDeleteShader(fragShader);
			glDeleteProgram(prog);
			return -1;
		}

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		*outProg = prog;
		return 0;
	}
}
}

