/*
 * ceeMPPM
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

#include <cee/platform/gfx.h>
#include <config.h>
#include <gfx_drm.h>
#include <gfx_egl_x.h>
#include <log.h>

#include <stddef.h>
#include <stdlib.h>

#include <memory>

namespace cee {
namespace platform {
	std::unique_ptr<GraphicsContext> GraphicsContext::Create(GfxContextType ctxType, Logger logger) {
		switch (ctxType) {
			case GfxContextType::PLATFORM_GFX_CONTEXT_X11:
#if defined(BUILD_PLATFORM_X11) && BUILD_PLATFORM_X11
				return std::unique_ptr<X11GraphicsContext>(new X11GraphicsContext(ctxType, logger));
#else
				error(logger, "Cannot use X11 context. Not built in this version");
				return nullptr;
#endif
			case GfxContextType::PLATFORM_GFX_CONTEXT_DRM:
#if defined(BUILD_PLATFORM_DRM) && BUILD_PLATFORM_DRM
				return std::unique_ptr<DRMGraphicsContext>(new DRMGraphicsContext(ctxType, logger));
#else
				error(logger, "Cannot use DRM context. Not built in this version");
				return nullptr;
#endif
			default:
				error(logger, "Invalid graphics backend!");
				return nullptr;
		}
	}
}
}

