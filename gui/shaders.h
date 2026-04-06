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

#include <string_view>

enum class GLSLVersion {
	ES_100,
	ES_320,
};

inline GLSLVersion GLSLSVersionFromString(std::string_view versionString) {
	if (versionString.find("OpenGL ES 3.2") != std::string_view::npos) {
		return GLSLVersion::ES_320;
	} else if (versionString.find("OpenGL ES 2.0") != std::string_view::npos) {
		return GLSLVersion::ES_100;
	}
	return GLSLVersion::ES_100;
}

constexpr std::string_view vertexShaderSourceES2 =
	"#version 100\n"
	"attribute vec4 aPosition;\n"
	"attribute vec4 aColor;\n"
	"\n"
	"uniform mat4 uPrpj;\n"
	"uniform mat4 uTrans;\n"
	"\n"
	"varying vec4 vColor;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = uProj * uTrans * aPosition;\n"
	"	vColor = aColor;\n"
	"}\n";
constexpr std::string_view fragmentShaderSourceES2 =
	"#version 100\n"
	"precision mediump float;\n"
	"\n"
	"varying vec4 vColor;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vColor;\n"
	"}\n";
constexpr std::string_view vertexShaderSourceES3 =
	"#version 320 es\n"
	"\n"
	"layout (location = 0) in vec4 aPosition;\n"
	"layout (location = 1) in vec4 aColor;\n"
	"\n"
	"uniform mat4 uProj;\n"
	"uniform mat4 uTrans;\n"
	"\n"
	"out vec4 vColor;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = uProj * uTrans * aPosition;\n"
	"	vColor = aColor;\n"
	"}\n";
constexpr std::string_view fragmentShaderSourceES3 =
	"#version 320 es\n"
	"\n"
	"precision mediump float;\n"
	"\n"
	"in vec4 vColor;\n"
	"\n"
	"out vec4 fragColor;\n"
	"\n"
	"void main() {\n"
	"	fragColor = vColor;\n"
	"}\n";

#endif
