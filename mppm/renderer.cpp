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

#include <cee/mppm/renderer.h>
#include <cee/mppm/log.h>
#include <config.h>
#if BUILD_GLES
#include <glesRenderer.h>
#elif BUILD_GL
#include <glRenderer.h>
#endif

namespace cee {
std::unique_ptr<Renderer> Renderer::Create() {
#if BUILD_GLES
	return std::make_unique<GLESRenderer>();
#elif BUILD_GL
	return std::make_unique<OpenGLRenderer>();
#else
	static_assert(0, "No renderer set, use -DBUILD_GLES or -DBUILD_GL");
	return nullptr;
#endif
}
}

