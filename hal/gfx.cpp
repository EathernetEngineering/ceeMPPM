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

#include <cee/hal/gfx.h>
#include <cee/hal/hal.h>
#include <gfx_drm.h>
#include <gfx_egl_x.h>
#include <log.h>
#include <util.h>
#include <config.h>

#include <stddef.h>
#include <stdlib.h>

#include <memory>

namespace cee {
namespace hal {
	std::unique_ptr<GraphicsContext> GraphicsContext::Create() {
		switch (GetGfxBackend()) {
#if defined(BUILD_HAL_X11) && BUILD_HAL_X11
			case HAL_GFX_BACKEND_X11:
				return std::unique_ptr<X11GraphicsContext>(new X11GraphicsContext());
#endif
#if defined(BUILD_HAL_DRM) && BUILD_HAL_DRM
			case HAL_GFX_BACKEND_DRM:
				return std::unique_ptr<DRMGraphicsContext>(new DRMGraphicsContext());
#endif
			default:
				CEE_ERROR("Invalid graphics backend!");
				return nullptr;
		}
	}
}
}

