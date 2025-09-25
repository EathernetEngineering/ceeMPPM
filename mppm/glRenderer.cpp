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

#include <glRenderer.h>
#include <cee/mppm/log.h>

#include <cee/hal/gfx.h>

#include <glad/gl.h>
#include <glad/glx.h>
#include <glm/ext/matrix_transform.hpp>

#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#define MAX_INDICES  16384

namespace cee {
OpenGLRenderer::OpenGLRenderer()
{
	GLenum ec;
	
	m_HALGfx = HALGfxCreate();
	if (m_HALGfx == NULL) {
		CEE_CORE_ERROR("Failed to allocate graphics backend!");
		throw std::runtime_error("Failed to allocate graphics backend!");
	}

	if (HALGfxInit(m_HALGfx)) {
		HALGfxDestroy(m_HALGfx);
		CEE_CORE_ERROR("Failed to initialize graphics backend!");
		throw std::runtime_error("Failed to initialize graphics backend!");
	}

	m_VersionString = HALGfxGetVersionString(m_HALGfx);
	CEE_CORE_INFO("Using OpenGL {}", m_VersionString);

	glViewport(0, 0, HALGfxGetWidth(m_HALGfx), HALGfxGetHeight(m_HALGfx));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_Triangles.vertices = reinterpret_cast<float*>(calloc(MAX_INDICES, sizeof(Vertex)));
	m_Triangles.indices = reinterpret_cast<uint16_t*>(calloc(MAX_INDICES, sizeof(uint16_t)));
	m_Quads.vertices = reinterpret_cast<float*>(calloc((MAX_INDICES * 4) / 6, sizeof(Vertex)));
	m_Quads.indices = reinterpret_cast<uint16_t*>(calloc(MAX_INDICES, sizeof(uint16_t)));
	if (!m_Triangles.vertices || !m_Triangles.indices || !m_Quads.vertices || !m_Quads.indices) {
		CEE_CORE_ERROR("Out of memory!");
		return;
	}

	GLuint buffers[4] = { 0 };
	// clear errors 
	glGetError();

	glGenBuffers(4, buffers);
	m_Triangles.vbo = buffers[0];
	m_Triangles.ebo = buffers[1];
	m_Quads.vbo = buffers[2];
	m_Quads.ebo = buffers[3];

	if ((ec = glGetError()) != GL_NO_ERROR) {
		CEE_CORE_ERROR("GL error: {0}", ec);
		throw std::runtime_error("GL error");
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_Triangles.vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_INDICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Triangles.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(uint16_t), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_Quads.vbo);
	glBufferData(GL_ARRAY_BUFFER, ((MAX_INDICES * 4) / 6) * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Quads.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(uint16_t), nullptr, GL_DYNAMIC_DRAW);

	if ((ec = glGetError()) != GL_NO_ERROR) {
		CEE_CORE_ERROR("GL error: {0}", ec);
		throw std::runtime_error("GL error");
	}

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(uintptr_t)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(uintptr_t)(4 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(uintptr_t)(8 * sizeof(float)));
	glEnableVertexAttribArray(2);

	m_Triangles.vertexCount = 0;
	m_Triangles.indexCount = 0;
	m_Quads.vertexCount = 0;
	m_Quads.indexCount = 0;
}

OpenGLRenderer::~OpenGLRenderer() {
	// eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	// eglDestroySurface(m_Display, m_Surface);
	// eglDestroyContext(m_Display, m_Context);
	if (m_HALGfx) {
		HALGfxShutdown(m_HALGfx);
		HALGfxDestroy(m_HALGfx);
	}
}

void OpenGLRenderer::SwapBuffers() {
	// EGLBoolean ret = eglSwapBuffers(m_Display, m_Surface);
	// if (ret != GL_TRUE) {
	// 	CEE_CORE_ERROR("eglSwapBuffers failed: {0}", eglGetError());
	// }
	HALGfxPageFlip(m_HALGfx);
}

void OpenGLRenderer::Clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::ClearColor(const glm::vec4 &color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRenderer::SetColor(const glm::vec4 &color) {
	m_ObjColor = color;
}

void OpenGLRenderer::DrawQuad(const glm::vec3 &position, const glm::vec3 &scale) {
	const static glm::vec4 positions[4] = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 1.0f }
	};
	const static glm::vec2 uvs[4] = {
		{ 0.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
	};

	glm::mat4 translatemat = glm::translate(position);
	glm::mat4 scalemat = glm::scale(scale);
	glm::mat4 transform = translatemat * scalemat;

	if (((m_Quads.indexCount + 6) > MAX_INDICES) || ((m_Quads.vertexCount + 4) > ((MAX_INDICES * 4) / 6))) {
		Flush();
	}
	
	m_Quads.indices[m_Quads.indexCount + 0] = m_Quads.vertexCount + 0;
	m_Quads.indices[m_Quads.indexCount + 1] = m_Quads.vertexCount + 1;
	m_Quads.indices[m_Quads.indexCount + 2] = m_Quads.vertexCount + 2;
	m_Quads.indices[m_Quads.indexCount + 3] = m_Quads.vertexCount + 2;
	m_Quads.indices[m_Quads.indexCount + 4] = m_Quads.vertexCount + 3;
	m_Quads.indices[m_Quads.indexCount + 5] = m_Quads.vertexCount + 0;
	m_Quads.indexCount += 6;

	for (int i = 0; i < 4; i++) {
		Vertex v;
		v.position = translatemat * scalemat * positions[i];
		v.color = m_ObjColor;
		v.uv = uvs[i];
		memcpy(reinterpret_cast<void*>((uintptr_t)(m_Quads.vertices) + (m_Quads.vertexCount * sizeof(Vertex))), &v, sizeof(Vertex));
		m_Quads.vertexCount++;
	}
}

void OpenGLRenderer::Flush() {
	if (m_Triangles.indexCount > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, m_Triangles.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_Triangles.vertexCount * sizeof(Vertex), m_Triangles.vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Triangles.ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Triangles.indexCount * sizeof(uint16_t), m_Triangles.indices);

		glDrawElements(GL_TRIANGLES, m_Triangles.indexCount, GL_UNSIGNED_SHORT, nullptr);

		m_Triangles.indexCount = 0;
		m_Triangles.vertexCount = 0;
	}

	if (m_Quads.indexCount > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, m_Quads.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_Quads.vertexCount * sizeof(Vertex), m_Quads.vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Quads.ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Quads.indexCount * sizeof(uint16_t), m_Quads.indices);

		glDrawElements(GL_TRIANGLES, m_Quads.indexCount, GL_UNSIGNED_SHORT, nullptr);

		m_Quads.indexCount = 0;
		m_Quads.vertexCount = 0;
	}

}

void OpenGLRenderer::StartFrame() {
	glViewport(0, 0, HALGfxGetWidth(m_HALGfx), HALGfxGetHeight(m_HALGfx));
}

void OpenGLRenderer::EndFrame() {
	Flush();
}
}

