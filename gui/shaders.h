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

#ifndef CEE_GUI_SHADERS_H_
#define CEE_GUI_SHADERS_H_

#include <config.h>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace cee {
namespace gui {
	// enum class GLSLVersion {
	// 	ES_100,
	// 	ES_320,
	// };
	//
	// inline GLSLVersion GLSLSVersionFromString(std::string_view versionString) {
	// 	if (versionString.find("OpenGL ES 3.2") != std::string_view::npos) {
	// 		return GLSLVersion::ES_320;
	// 	} else if (versionString.find("OpenGL ES 2.0") != std::string_view::npos) {
	// 		return GLSLVersion::ES_100;
	// 	}
	// 	return GLSLVersion::ES_100;
	// }

	class Shader {
	public:
		Shader(std::string_view vertSrc, std::string_view fragSrc);
		~Shader();

		void Bind();

		void SetUniform(const std::string& name, const glm::mat4& value);

	private:
		GLint GetUniformLocation(const std::string& name);

	private:
		GLint m_Program;
		std::unordered_map<std::string, GLint> m_UniformLocations;
	};
}
}

#endif
