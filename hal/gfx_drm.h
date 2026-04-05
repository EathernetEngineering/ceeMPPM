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

#ifndef CEE_HAL_GFX_DRM_H_
#define CEE_HAL_GFX_DRM_H_

#include <cee/hal/gfx.h>
#include <config.h>

#include <glad/egl.h>
#include <glad/gles2.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

namespace cee {
namespace hal {
	class DRMGraphicsContext : public GraphicsContext {
		protected:
			DRMGraphicsContext();

		public:
			virtual ~DRMGraphicsContext();

			virtual void Init() override;
			virtual void Shutdown() override;

			virtual const char* GetVersionString() const override;
			virtual const char* GetShadingVersionString() const override;
			virtual void SwapBuffers() override;

		protected:
			EGLDisplay m_EglDisplay;
			EGLConfig m_EglConfig;
			EGLContext m_EglContext;
			EGLSurface m_EglSurface;

			struct display {
				int fd;
				drmModeRes *resources;
				drmModeConnector *connector;
				drmModeEncoder *encoder;
				drmModeModeInfo *connectorMode;
				uint32_t connectorId;
				uint32_t crtcIndex;
				uint32_t crtcId;
				drmEventContext eventContext;

				int width, height;
			} m_DRMDisplay;

			struct GBMFramebuffer {
				gbm_device *device;;
				gbm_surface *surface;;
				gbm_bo *fbo;
				gbm_bo *bbo;
				uint32_t fboId;

				int width, height;
				uint32_t format;
			} m_FB;

			gbm_bo *m_OldFBO;

		protected:
			void ChooseDRMDevice();
			void ChooseConnector();
			int ChooseCrtcForEncoder(const drmModeEncoder *encoder) const;
			int ChooseCrtcForConnector(const drmModeConnector *connector) const;
			void ChooseConnectorMode();
			void DRMPageFlip();
			void GetDRMModeID();

			void CreateGBMSurface();
			void GBMPageFlip();
			void GBMReleaseBuffer();

			void ChooseEGLConfig(EGLint attribs[]);
			int MatchEGLConfigToVisual(EGLConfig configs[], int count) const;

		public:
			friend std::unique_ptr<GraphicsContext> GraphicsContext::Create();
	};
}
}

#endif

