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

#include <gfx_drm.h>
#include <log.h>
#include <config.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include <gbm.h>

#include <glad/egl.h>
#include <glad/gles2.h>

#include <fcntl.h>
#include <cstring>

#define MAX_DRM_DEVICES 64

struct gbmFbInfo {
	struct gbm_bo *bo;
	uint32_t id;
};

static void destroyUserDataCallback(gbm_bo *, void *data)
{
	free(data);
}

namespace cee {
namespace hal {
	DRMGraphicsContext::DRMGraphicsContext()
	 : m_EglDisplay(EGL_NO_DISPLAY), m_EglContext(EGL_NO_CONTEXT), m_EglSurface(EGL_NO_SURFACE)
	{
		memset(&m_DRMDisplay, 0, sizeof(m_DRMDisplay));
		memset(&m_FB, 0, sizeof(m_FB));
	}

	DRMGraphicsContext::~DRMGraphicsContext()
	{
		if (m_DRMDisplay.fd > 0)
			Shutdown();
	}

	void DRMGraphicsContext::Init()
	{
		EGLBoolean result = EGL_FALSE;
		GLenum ec;

		ChooseDRMDevice();
		ChooseConnector();
		ChooseConnectorMode();
		CEE_DEBUG("Chose connector 0x{:X}, {}x{}@{}Hz",
			m_DRMDisplay.connectorId,
			m_DRMDisplay.connectorMode->hdisplay,
			m_DRMDisplay.connectorMode->vdisplay,
			m_DRMDisplay.connectorMode->vrefresh);

		m_FB.device = gbm_create_device(m_DRMDisplay.fd);
		if (!m_FB.device) {
			CEE_ERROR("Failed to create gbm device");
			throw std::runtime_error("Failed to create gbm device");
		}
		CreateGBMSurface();

		gladLoaderLoadEGL(NULL);
		m_EglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, (void *)m_FB.device, NULL);

		if (m_EglDisplay == EGL_NO_DISPLAY) {
			CEE_ERROR("Failed to get platform EGLDisplay");
			throw std::runtime_error("Failed to get platform EGLDisplay");
		}

		EGLint major, minor;
		if (!(result = eglInitialize(m_EglDisplay, &major, &minor))) {
			CEE_ERROR("Failed to initialize EGL");
			throw std::runtime_error("Failed to initialize EGL");
		}

		gladLoaderLoadEGL(m_EglDisplay);
		CEE_DEBUG("Loaded EGL {}.{}:", major, minor);
		CEE_DEBUG("\tVENDOR: {}", eglQueryString(m_EglDisplay, EGL_VENDOR));
		CEE_DEBUG("\tDISPLAY: {:X}", (uint64_t)m_EglDisplay);

		if (!(result = eglBindAPI(EGL_OPENGL_ES_API))) {
			CEE_ERROR("Failed to bind EGL OpenGL API");
			throw std::runtime_error("Failed to bind EGL OpenGL API");
		}

		static const EGLint contextAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
		EGLint configAttribs[] = {
			EGL_RED_SIZE, 1,
			EGL_GREEN_SIZE, 1,
			EGL_BLUE_SIZE, 1,
			EGL_ALPHA_SIZE, 0,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_NONE
		};

		ChooseEGLConfig(configAttribs);

		m_EglContext = eglCreateContext(m_EglDisplay, m_EglConfig, EGL_NO_CONTEXT, contextAttribs);
		if (m_EglContext == EGL_NO_CONTEXT) {
			CEE_ERROR("Failed to create EGL context: 0x{:X}", eglGetError());
			throw std::runtime_error("Failed to create EGL context");
		}

		m_EglSurface = eglCreatePlatformWindowSurfaceEXT(m_EglDisplay,
				m_EglConfig,
				m_FB.surface,
				NULL);

		if (m_EglSurface == EGL_NO_SURFACE) {
			CEE_ERROR("Failed to create EGL surface: 0x{:X}", eglGetError());
			throw std::runtime_error("Failed to create EGL surface");
		}

		if (!eglMakeCurrent(m_EglDisplay, m_EglSurface, m_EglSurface, m_EglContext)) {
			CEE_ERROR("Failed to make context current: 0x{:X}", eglGetError());
			throw std::runtime_error("Failed to make context current");
		}

		int glVersion;

		if ((glVersion = gladLoaderLoadGLES2()) == 0) {
			CEE_ERROR("Failed to load OpenGL ES");
			throw std::runtime_error("Failed to load OpenGL ES");
		}

		CEE_DEBUG("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(glVersion), GLAD_VERSION_MINOR(glVersion));
		CEE_DEBUG("\tVersion: {}", this->GetVersionString());
		CEE_DEBUG("\tShading Langauge Version: {}", this->GetShadingVersionString());
		CEE_DEBUG("\tRenderer: {}", (const char *)glGetString(GL_RENDERER));

		glClearColor(.0f, .0f, .0f, .0f);
		glClear(GL_COLOR_BUFFER_BIT);

		eglSwapBuffers(m_EglDisplay, m_EglSurface);

		GBMPageFlip();

		result = drmModeSetCrtc(m_DRMDisplay.fd,
				m_DRMDisplay.crtcId,
				m_FB.fboId,
				0, 0,
				&m_DRMDisplay.connectorId,
				1,
				m_DRMDisplay.connectorMode);

		if (result) {
			CEE_DEBUG("drmModeSetCrtc: {}", strerror(errno));
			result = drmIsMaster(m_DRMDisplay.fd);
			if (!result) {
				CEE_DEBUG("Attempting to set drm master...");
				result = drmSetMaster(m_DRMDisplay.fd);
				if (result < 0 || !drmIsMaster(m_DRMDisplay.fd)) {
					CEE_ERROR("... Fail");
					throw std::runtime_error("Failed to set drm master");
				}
				CEE_DEBUG("... Success");

				result = drmModeSetCrtc(m_DRMDisplay.fd,
									 m_DRMDisplay.crtcId,
									 m_FB.fboId,
									 0, 0,
									 &m_DRMDisplay.connectorId,
									 1,
									 m_DRMDisplay.connectorMode);
				if (result < 0) {
					CEE_DEBUG("drmModeSetCrtc: {}", strerror(errno));
					throw std::runtime_error("Unable to set crtc");
				}
				CEE_DEBUG("Crtc set");
			} else {
				CEE_ERROR("Unable to set crtc");
				throw std::runtime_error("Unable to set crtc");
			}
		}

		DRMPageFlip();

		GBMReleaseBuffer();
	}

	void DRMGraphicsContext::Shutdown()
	{
		eglMakeCurrent(m_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroySurface(m_EglDisplay, m_EglSurface);
		m_EglSurface = EGL_NO_SURFACE;
		eglDestroyContext(m_EglDisplay, m_EglContext);
		m_EglContext = EGL_NO_CONTEXT;
		eglTerminate(m_EglDisplay);
		m_EglDisplay = EGL_NO_DISPLAY;

		// TODO ASAP: Cleanup GBM resources and DRM resources
	}

	const char* DRMGraphicsContext::GetVersionString() const
	{
		return reinterpret_cast<const char *>(glGetString(GL_VERSION));
	}

	const char* DRMGraphicsContext::GetShadingVersionString() const
	{
		return reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	void DRMGraphicsContext::SwapBuffers()
	{
		if (eglSwapBuffers(m_EglDisplay, m_EglSurface) != GL_TRUE) {
			CEE_ERROR("eglSwapBuffers failed: 0x{:X}", eglGetError());
			throw std::runtime_error("eglSwapBuffers failed");
		}
		
		GBMPageFlip();
		DRMPageFlip();
		GBMReleaseBuffer();
	}

	void DRMGraphicsContext::ChooseDRMDevice()
	{
		drmDevicePtr devices[MAX_DRM_DEVICES] = { NULL };
		int countDevices;
		m_DRMDisplay.fd = -1;

		countDevices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
		if (countDevices < 0) {
			CEE_ERROR("drmGetDevices2 failed ({}): {}", -countDevices, strerror(-countDevices));
			throw std::runtime_error("drmGetDevices2 failed");
		}

		for (int i = 0; i < countDevices; i++) {
			drmDevicePtr device = devices[i];
			int ret;

			if (!(device->available_nodes & (1 << DRM_NODE_PRIMARY))) {
				continue;
			}

			m_DRMDisplay.fd = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR);
			if (m_DRMDisplay.fd < 0) {
				continue;
			}

			m_DRMDisplay.resources = drmModeGetResources(m_DRMDisplay.fd);
			if (m_DRMDisplay.resources) {
				CEE_DEBUG("Using device {}", device->nodes[DRM_NODE_PRIMARY]);
				break;
			} else {
				CEE_WARN("drmModeGetResources failed");
			}

			close(m_DRMDisplay.fd);
			m_DRMDisplay.fd = -1;
		}
		drmFreeDevices(devices, countDevices);

		if (m_DRMDisplay.fd < 0) {
			CEE_ERROR("No drm device found!");
			throw std::runtime_error("No drm device found");
		}

		m_DRMDisplay.eventContext.version = 2;
		m_DRMDisplay.eventContext.page_flip_handler = [](int, uint32_t, uint32_t, uint32_t, void* data){ *((int*)data) = 0; };
	}

	void DRMGraphicsContext::ChooseConnector()
	{
		if (m_DRMDisplay.connectorId >= 0) {
			if (m_DRMDisplay.connectorId >= m_DRMDisplay.resources->count_connectors) {
				CEE_ERROR("Provided connector ID invlalid. Given {}. connector count: {}",
						m_DRMDisplay.connectorId, m_DRMDisplay.resources->count_connectors);
				throw std::runtime_error("Provided connector ID invlalid");
			}

			m_DRMDisplay.connector = drmModeGetConnector(m_DRMDisplay.fd, m_DRMDisplay.resources->connectors[m_DRMDisplay.connectorId]);
			if (m_DRMDisplay.connector == NULL) {
				CEE_ERROR("Provided m_DRMDisplay.connector ID invlalid");
				throw std::runtime_error("Provided connector ID invlalid");
			}
			if (m_DRMDisplay.connector->connection != DRM_MODE_CONNECTED) {
				CEE_ERROR("Provided m_DRMDisplay.connector ID is not connected");
				drmModeFreeConnector(m_DRMDisplay.connector);
				throw std::runtime_error("Provided connector ID is not connected");
			}
			return;
		}

		for (int i = 0; i < m_DRMDisplay.resources->count_connectors; i++) {
			m_DRMDisplay.connector = drmModeGetConnector(m_DRMDisplay.fd, m_DRMDisplay.resources->connectors[i]);
			if (m_DRMDisplay.connector == NULL) {
				continue;
			}
			if (m_DRMDisplay.connector->connection == DRM_MODE_CONNECTED) {
				break;
			}
			drmModeFreeConnector(m_DRMDisplay.connector);
			m_DRMDisplay.connector = NULL;
		}
		if (m_DRMDisplay.connector == NULL)
			throw std::runtime_error("No connected connector found");
	}

	int DRMGraphicsContext::ChooseCrtcForEncoder(const drmModeEncoder *encoder) const
	{
		for (int i = 0; i < m_DRMDisplay.resources->count_crtcs; i++) {
			const uint32_t crtcMask = 1 << i;
			const uint32_t crtcId = m_DRMDisplay.resources->crtcs[i];

			if (encoder->possible_crtcs & crtcMask) {
				return crtcId;
			}
		}

		return -1;
	}

	int DRMGraphicsContext::ChooseCrtcForConnector(const drmModeConnector *connector) const
	{
		for (int i = 0; i < connector->count_encoders; i++) {
			const uint32_t encoderId = connector->encoders[i];
			drmModeEncoder *encoder = drmModeGetEncoder(m_DRMDisplay.fd, encoderId);

			if (encoder) {
				const int32_t crtcId = ChooseCrtcForEncoder(encoder);
				drmModeFreeEncoder(encoder);

				if (crtcId != 0) {
					return crtcId;
				}
			}
		}

		return -1;
	}

	void DRMGraphicsContext::ChooseConnectorMode()
	{
		int area;
		for (int i = 0; i < m_DRMDisplay.connector->count_modes; i++) {
			drmModeModeInfo *currentMode = &m_DRMDisplay.connector->modes[i];

			if (currentMode->type & DRM_MODE_TYPE_PREFERRED) {
				m_DRMDisplay.connectorMode = currentMode;
				break;
			}
			int currentArea = currentMode->hdisplay * currentMode->vdisplay;
			if (currentArea > area) {
				m_DRMDisplay.connectorMode = currentMode;
				area = currentArea;
			}
		}

		if (!m_DRMDisplay.connectorMode) {
			CEE_ERROR("No valid mode found for connector!");
			throw std::runtime_error("No valid mode found for connector");
		}

		m_DRMDisplay.width = m_DRMDisplay.connectorMode->hdisplay;
		m_DRMDisplay.height = m_DRMDisplay.connectorMode->vdisplay;

		for (int i = 0; i < m_DRMDisplay.resources->count_encoders; i++) {
			m_DRMDisplay.encoder = drmModeGetEncoder(m_DRMDisplay.fd, m_DRMDisplay.resources->encoders[i]);
			if (m_DRMDisplay.encoder->encoder_id == m_DRMDisplay.connector->encoder_id) {
				break;
			}
			drmModeFreeEncoder(m_DRMDisplay.encoder);
			m_DRMDisplay.encoder = NULL;
		}

		if (m_DRMDisplay.encoder) {
			m_DRMDisplay.crtcId = m_DRMDisplay.encoder->crtc_id;
		} else {
			int crtcId = ChooseCrtcForConnector(m_DRMDisplay.connector);
			if (crtcId == -1) {
				CEE_ERROR("No crtc found!");
				throw std::runtime_error("No crtc found");
			}
			m_DRMDisplay.crtcId = crtcId;
		}

		for (int i = 0; i < m_DRMDisplay.resources->count_crtcs; i++) {
			if (m_DRMDisplay.resources->crtcs[i] == m_DRMDisplay.crtcId) {
				m_DRMDisplay.crtcIndex = i;
				break;
			}
		}
		drmModeFreeResources(m_DRMDisplay.resources);

		m_DRMDisplay.connectorId = m_DRMDisplay.connector->connector_id;
	}

	void DRMGraphicsContext::DRMPageFlip()
	{
		int waitingForFlip = 1, result = -1;
		fd_set fds;
		if ((result = drmModePageFlip(m_DRMDisplay.fd, m_DRMDisplay.crtcId, m_FB.fboId, DRM_MODE_PAGE_FLIP_EVENT, &waitingForFlip))) {
			CEE_DEBUG("drmModePageFlip: {}", strerror(errno));
			CEE_DEBUG("Drm page flipped failed, releasing buffer");

			drmModeRmFB(m_DRMDisplay.fd, m_FB.fboId);
			GBMReleaseBuffer();
			throw std::runtime_error("Page flip failed");
		}

		while (waitingForFlip) {
			FD_ZERO(&fds);
			FD_SET(m_DRMDisplay.fd, &fds);

			result = select(m_DRMDisplay.fd + 1, &fds, NULL, NULL, NULL);
			if (result < 0) {
			} else if (result == 0) {
				CEE_WARN("select timeout");
				return;
			} else {
				drmHandleEvent(m_DRMDisplay.fd, &m_DRMDisplay.eventContext);
			}
		}
	}

	void DRMGraphicsContext::GetDRMModeID()
	{
		gbmFbInfo *fbInfo;
		int fd;
		fbInfo = reinterpret_cast<gbmFbInfo *>(gbm_bo_get_user_data(m_FB.bbo));
		if (fbInfo) {
			m_FB.fbo = fbInfo->bo;
			m_FB.fboId = fbInfo->id;

			return;
		}
		fd = gbm_device_get_fd(m_FB.device);

		fbInfo = reinterpret_cast<gbmFbInfo *>(calloc(1, sizeof(gbmFbInfo)));
		if (!fbInfo)
			throw std::runtime_error("Failed to allocate framebuffer info");

		m_FB.fbo = m_FB.bbo;
		m_FB.fboId = 0;


		uint32_t strides[4] = { 0 };
		uint32_t offsets[4] = { 0 };
		uint32_t handles[4] = { 0 };
		uint32_t format = 0;
		uint32_t width = 0, height = 0;
		uint32_t flags = 0;
		int32_t result = -1;

		uint64_t modifiers[4] = { 0 };
		width = gbm_bo_get_width(m_FB.bbo);
		height = gbm_bo_get_height(m_FB.bbo);
		format = gbm_bo_get_format(m_FB.bbo);
		modifiers[0] = gbm_bo_get_modifier(m_FB.bbo);
		const int planeCount = gbm_bo_get_plane_count(m_FB.bbo);
		for (uint32_t i = 0; i < planeCount; i++) {
			strides[i] = gbm_bo_get_stride_for_plane(m_FB.bbo, i);
			offsets[i] = gbm_bo_get_offset(m_FB.bbo, i);
			handles[i] = gbm_bo_get_handle_for_plane(m_FB.bbo, i).u32;
			modifiers[i] = modifiers[0];
		}
		if (modifiers[0] && (modifiers[0] != DRM_FORMAT_MOD_INVALID)) { 
			flags = DRM_MODE_FB_MODIFIERS;
				CEE_DEBUG("drmModeAddFB2WithModifiers with modifiers: %llu", modifiers[0]);
		}

		result = drmModeAddFB2WithModifiers(fd,
								width, height,
								format,
								handles,
								strides, offsets,
								modifiers,
								&m_FB.fboId, flags);

		if (result)  {
			if (flags)
				CEE_DEBUG("drmModeAddFB2WithModifiers failed: %s", strerror(errno));

			handles[0] = gbm_bo_get_handle(m_FB.bbo).u32;
			strides[0] = gbm_bo_get_stride(m_FB.bbo);
			memset(&strides[1], 0, sizeof(uint32_t)*3);
			memset(&offsets[0], 0, sizeof(uint32_t)*4);
			memset(&handles[1], 0, sizeof(uint32_t)*3);

			result = drmModeAddFB2(fd,
							  m_FB.width, m_FB.height,
							  format,
							  handles,
							  strides, offsets,
							  &m_FB.fboId, 0);

			if (result) {
				CEE_ERROR("Fallback failed: drmModeAddFB2 ({}): {}", errno, strerror(errno));
				CEE_DEBUG("fd = {}, width = {}, height = {}, format = {}, handle = {}",
						fd, m_FB.width, m_FB.height, format, handles[0]);
				CEE_DEBUG("stride = {}, offset = {}, drmBoId = {}, flags = 0", strides[0], offsets[0], m_FB.fboId);
				delete fbInfo;
				throw std::runtime_error("Failed to create framebuffer");
			}
			fbInfo->bo = m_FB.bbo;
			fbInfo->id = m_FB.fboId;

			gbm_bo_set_user_data(m_FB.bbo, fbInfo, destroyUserDataCallback);
		}
	}

	void DRMGraphicsContext::CreateGBMSurface()
	{
		m_Width = m_FB.width = m_DRMDisplay.width;
		m_Height = m_FB.height = m_DRMDisplay.height;
		m_FB.format = GBM_FORMAT_XRGB8888;

		uint64_t modifiers[] = { DRM_FORMAT_MOD_LINEAR };

		m_FB.surface = gbm_surface_create_with_modifiers2(m_FB.device,
				m_FB.width, m_FB.height, m_FB.format,
				modifiers, (sizeof(modifiers)/sizeof(modifiers[0])),
				GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

		if (!m_FB.surface) {
			CEE_ERROR("Failed to create gbm surface");
			m_OldFBO = nullptr;
			throw std::runtime_error("Failed to create gbm surface");
		}
	}

	void DRMGraphicsContext::GBMPageFlip()
	{
		m_FB.bbo = gbm_surface_lock_front_buffer(m_FB.surface);
		m_OldFBO = m_FB.fbo;
		GetDRMModeID();
	}

	void DRMGraphicsContext::GBMReleaseBuffer()
	{
		gbm_surface_release_buffer(m_FB.surface, m_OldFBO);
	}

	void DRMGraphicsContext::ChooseEGLConfig(EGLint attribs[])
	{
		EGLint count = 0;
		EGLint matched = 0;
		EGLConfig *configs;
		int configIndex = -1;

		if (!eglGetConfigs(m_EglDisplay, NULL, 0, &count)) {
			CEE_ERROR("No EGL configs to choose from");
			throw std::runtime_error("No EGL configs to choose from");
		}
		configs = (EGLConfig *)(calloc(count, sizeof(*configs)));
		if (configs == NULL) {
			CEE_ERROR("Failed to allocate configs");
			throw std::runtime_error("Failed to allocate configs");
		}

		if (!eglChooseConfig(m_EglDisplay, attribs, configs, count, &matched) || !matched) {
			CEE_ERROR("No EGL configs with approprriate attributes");
			free(configs);
			throw std::runtime_error("No EGL configs with appropriate attributes");
		}

		if (!m_FB.format)
			configIndex = 0;
		
		if (configIndex == -1)
			configIndex = MatchEGLConfigToVisual(configs, count);
		
		if (configIndex != -1)
			m_EglConfig = configs[configIndex];

		free(configs);

		if (configIndex == -1) {
			throw std::runtime_error("No EGL config found matching GBM format");
		}
	}

	int DRMGraphicsContext::MatchEGLConfigToVisual(EGLConfig configs[], int count) const {
		for (int i = 0; i < count; i++) {
			EGLint id;
			if (!eglGetConfigAttrib(m_EglDisplay, configs[i], EGL_NATIVE_VISUAL_ID, &id))
				continue;

			if (id == m_FB.format)
				return i;
		}
		return -1;
	}
}
}

