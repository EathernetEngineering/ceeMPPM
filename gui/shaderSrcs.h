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

#ifndef CEE_GUI_SHADERSRCS_H_
#define CEE_GUI_SHADERSRCS_H_

#include <config.h>

#include <string_view>

constexpr std::string_view VSColorV2 =
	"#version 100\n"
	"attribute vec4 aPosition;\n"
	"attribute vec4 aColor;\n"
	"attribute vec2 aUV;\n"
	"\n"
	"uniform mat4 uProj;\n"
	"\n"
	"varying vec4 vColor;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = uProj * aPosition;\n"
	"	vColor = aColor;\n"
	"}\n";
constexpr std::string_view FSColorV2 =
	"#version 100\n"
	"precision mediump float;\n"
	"\n"
	"varying vec4 vColor;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vColor;\n"
	"}\n";
constexpr std::string_view VSColorV3 =
	"#version 320 es\n"
	"\n"
	"layout (location = 0) in vec4 aPosition;\n"
	"layout (location = 1) in vec4 aColor;\n"
	"layout (location = 2) in vec2 aUV;\n"
	"\n"
	"uniform mat4 uProj;\n"
	"\n"
	"out vec4 vColor;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = uProj * aPosition;\n"
	"	vColor = aColor;\n"
	"}\n";

constexpr std::string_view FSColorV3 =
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

constexpr std::string_view VSTextV2 =
	"#version 100\n"
	"attribute vec4 aPosition;\n"
	"attribute vec4 aColor;\n"
	"attribute vec2 aUV;\n"
	"\n"
	"uniform mat4 uPrpj;\n"
	"\n"
	"varying vec4 vColor;\n"
	"varying vec2 vUV;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = uProj * aPosition;\n"
	"	vColor = aColor;\n"
	"	vUV = aUV;\n"
	"}\n";
constexpr std::string_view FSTextV2 =
	"#version 100\n"
	"precision mediump float;\n"
	"\n"
	"varying vec4 vColor;\n"
	"varying vec2 vUV;\n"
	"\n"
	"uniform sampler2D uSampler;\n"
	"\n"
	"void main() {\n"
	"	float coverage = texture2D(uSampler, vUV).r;\n"
	"	vec4 texelColor = vec4(vColor.rgb, vColor.a * coverage);\n"
	"	gl_FragColor = texelColor;\n"
	"}\n";
constexpr std::string_view VSTextV3 =
	"#version 320 es\n"
	"\n"
	"layout (location = 0) in vec4 aPosition;\n"
	"layout (location = 1) in vec4 aColor;\n"
	"layout (location = 2) in vec2 aUV;\n"
	"\n"
	"uniform mat4 uProj;\n"
	"\n"
	"out vec4 vColor;\n"
	"out vec2 vUV;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = uProj * aPosition;\n"
	"	vColor = aColor;\n"
	"	vUV = aUV;\n"
	"}\n";
constexpr std::string_view FSTextV3 =
	"#version 320 es\n"
	"\n"
	"precision mediump float;\n"
	"\n"
	"in vec4 vColor;\n"
	"in vec2 vUV;\n"
	"\n"
	"uniform sampler2D uSampler;\n"
	"\n"
	"out vec4 fragColor;\n"
	"\n"
	"void main() {\n"
	"	float coverage = texture(uSampler, vUV).r;\n"
	"	vec4 texelColor = vec4(vColor.rgb, vColor.a * coverage);\n"
	"	fragColor = texelColor;\n"
	"}\n";

#endif
