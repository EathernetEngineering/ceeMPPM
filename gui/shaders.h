/*
 * ceeGUI
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

#ifndef CEE_GUI_SHADERS_H_
#define CEE_GUI_SHADERS_H_

#include <config.h>
#include <cee/core/except.h>
#include <cee/core/log.h>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace cee {
namespace gui {
	class Context;

	class ShaderCompilerError : public core::Error {
	public:
		explicit ShaderCompilerError(const std::string &what) : Error(what) {}
		explicit ShaderCompilerError(const char *what) : Error(what) {}
		ShaderCompilerError(const ShaderCompilerError &other) = default;
		ShaderCompilerError &operator=(const ShaderCompilerError &other) = default;
	};

	class Shader {
	public:
		Shader(std::string_view vertSrc, std::string_view fragSrc, Logger logger);
		~Shader();

		void Bind();

		void SetUniform(const std::string& name, const glm::mat4& value);

	private:
		GLint GetUniformLocation(const std::string& name);

	private:
		GLint m_Program;
		Logger m_Logger;
		std::unordered_map<std::string, GLint> m_UniformLocations;
	};
}
}

#endif
