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

#include <shaders.h>
#include <log.h>
#include <cee/profiler/profiler.h>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

namespace cee {
namespace gui {
	static void PrettyPrintErrorLog(const char *log) {
		std::string_view logView(log);
		size_t pos = 0;
		while (pos < logView.size()) {
			size_t nextPos = logView.find('\n', pos);
			if (nextPos == std::string_view::npos) {
				nextPos = logView.size();
			}
			std::string_view line = logView.substr(pos, nextPos - pos);
			CEE_WARN("\t{}", line);
			pos = nextPos + 1;
		}
	}

	Shader::Shader(std::string_view vertSrc, std::string_view fragSrc)
	 : m_Program(0)
	{
		PROFILE_FUNCTION();
		GLuint vertShader, fragShader;
		GLint success;

		vertShader = glCreateShader(GL_VERTEX_SHADER);
		if (vertShader == 0)
			throw std::runtime_error("Failed to create vertex shader");
		fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		if (fragShader == 0)
			throw std::runtime_error("Failed to create fragment shader");

		{
			PROFILE_SCOPE("Compile vertex shader");
			const char *vertSrcs[] = { vertSrc.data() };
			glShaderSource(vertShader, 1, vertSrcs, NULL);
			glCompileShader(vertShader);
			glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				GLint logLength;
				char *log;
				glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
				log = new char[logLength];
				if (log == nullptr) {
					glDeleteShader(vertShader);
					throw std::runtime_error("Failed to allocate memory for vertex shader error log");
				}
				glGetShaderInfoLog(vertShader, logLength, NULL, log);
				CEE_WARN("Failed to compile vertex shader:");
				PrettyPrintErrorLog(log);
				
				delete[] log;
				glDeleteShader(vertShader);
				throw std::runtime_error("Failed to compile vertex shader");
			}
		}
		{
			PROFILE_SCOPE("Compile fragment shader");
			const char *fragSrcs[] = { fragSrc.data() };
			glShaderSource(fragShader, 1, fragSrcs, NULL);
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
					throw std::runtime_error("Failed to allocate memory for fragment shader error log");
				}
				glGetShaderInfoLog(fragShader, logLength, NULL, log);
				CEE_WARN("Failed to compile fragment shader:");
				PrettyPrintErrorLog(log);

				delete[] log;
				glDeleteShader(vertShader);
				glDeleteShader(fragShader);
				throw std::runtime_error("Failed to compile fragment shader");
			}
		}
		m_Program = glCreateProgram();
		if (m_Program == 0)
			throw std::runtime_error("Failed to create shader program");
		{
			PROFILE_SCOPE("Link shader program");
			glAttachShader(m_Program, vertShader);
			glAttachShader(m_Program, fragShader);
			glLinkProgram(m_Program);
			glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
			if (!success) {
				char *log;
				GLint logLength;
				glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLength);
				log = new char[logLength];
				if (log == nullptr) {
					glDeleteShader(vertShader);
					glDeleteShader(fragShader);
					glDeleteProgram(m_Program);
					throw std::runtime_error("Failed to allocate memory for shader program error log");
				}
				glGetProgramInfoLog(m_Program, logLength, NULL, log);
				CEE_WARN("Failed to link shader:");
				PrettyPrintErrorLog(log);

				delete[] log;
				glDeleteShader(vertShader);
				glDeleteShader(fragShader);
				glDeleteProgram(m_Program);
				throw std::runtime_error("Failed to link shader");
			}
		}

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
	}

	Shader::~Shader() {
		glDeleteProgram(m_Program);
	}

	void Shader::Bind() {
		glUseProgram(m_Program);
	}

	void Shader::SetUniform(const std::string& name, const glm::mat4& value) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	GLint Shader::GetUniformLocation(const std::string& name) {
		PROFILE_FUNCTION();
		auto it = m_UniformLocations.find(name);

		if (it == m_UniformLocations.end()) {
			GLint location = glGetUniformLocation(m_Program, name.c_str());
			if (location == -1) {
				CEE_WARN("Uniform '{}' not found in shader", name);
				throw std::runtime_error("Uniform not found in shader");
			}
			m_UniformLocations[name] = location;
			return location;
		}

		return it->second;
	}
}
}
