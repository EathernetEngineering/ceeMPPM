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

#include <context.h>
#include <cee/gui/object.h>
#include <log.h>
#include <shaderSrcs.h>

#include <cee/profiler/profiler.h>

#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

#include <stdexcept>

namespace cee {
namespace gui {
	Context::Context() {
		 m_FontManager = std::make_unique<font::FontManager>();
		 m_FontManager->SetDPI(96);
		 m_Fonts.emplace(m_Fonts.begin(), m_FontManager->CreateFont("/usr/share/fonts/Adwaita/AdwaitaSans-Regular.ttf"));

		glGenBuffers(1, &m_VBO);
		if (m_VBO == 0) {
			throw std::runtime_error("Failed to create OpenGL vertex buffer object");
		}
		glGenBuffers(1, &m_EBO);
		if (m_EBO == 0) {
			throw std::runtime_error("Failed to create OpenGL index buffer object");
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(2);

		glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1024, nullptr, GL_DYNAMIC_DRAW);
		
		try {
			m_QuadFlatShader = std::make_unique<Shader>(VSColorV2, FSColorV2);
			CEE_DEBUG("Flat color shader compiled successfully");
			m_TextShader = std::make_unique<Shader>(VSTextV2, FSTextV2);
			CEE_DEBUG("Text shader compiled successfully");
		} catch (const std::exception& e) {
			CEE_TRACE("Failed to compile GLSL ES 100 shaders: {}", e.what());
			CEE_TRACE("Trying GLSL ES 320");

			m_QuadFlatShader = std::make_unique<Shader>(VSColorV3, FSColorV3);
			CEE_DEBUG("Flat color shader compiled successfully");
			m_TextShader = std::make_unique<Shader>(VSTextV3, FSTextV3);
			CEE_DEBUG("Text shader compiled successfully");
		}
		m_CurrentShader = GuiShader::Flat;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		m_Projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);
	}

	Context::~Context() {
		for (auto& tex : m_TextTextures) {
			glDeleteTextures(1, &tex.name);
		}
		glUseProgram(GL_NONE);
		m_QuadFlatShader.reset();
		m_TextShader.reset();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
		m_Fonts.clear();
		m_FontManager.reset();
	}

	void Context::SetViewport(const Size &viewport) {
		if (viewport.w == m_Viewport.w && viewport.h == m_Viewport.h)
			return;
		m_Viewport = viewport;
		m_Projection = glm::ortho(0.f, m_Viewport.w, m_Viewport.h, 0.f);
	}

	void Context::PushClip(const Rect &clip) {
		Flush();
		// TODO: Set scissor rect to clip
		m_ClipStack.push(clip);
	}

	void Context::PopClip() {
		Flush();
		m_ClipStack.pop();
	}

	void Context::PushTransform(const Size &transform) {
		Flush();
		m_TransformStack.push(transform);
		// TODO:
	}

	void Context::PopTransform() {
		Flush();
		m_TransformStack.pop();
	}

	void Context::DrawTriangle(const Point &a, const Point &b, const Point &c, const Color &color)
	{
		if (m_Triangles.vertexCount + 3 > BATCH_MAX_VERTICES) {
			FlushTriangles();
		}
		m_Triangles.vertices[m_Triangles.vertexCount++] = {
			{ a.x, a.y, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Triangles.vertices[m_Triangles.vertexCount++] = {
			{ b.x, b.y, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Triangles.vertices[m_Triangles.vertexCount++] = {
			{ c.x, c.y, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Triangles.indices[m_Triangles.indexCount++] = m_Triangles.vertexCount - 3;
		m_Triangles.indices[m_Triangles.indexCount++] = m_Triangles.vertexCount - 2;
		m_Triangles.indices[m_Triangles.indexCount++] = m_Triangles.vertexCount - 1;
	}

	void Context::DrawRect(const Rect& rect, const Color& color) {
		if (m_Rects.vertexCount + 4 > BATCH_MAX_VERTICES) {
			FlushRects();
		}
		m_Rects.vertices[m_Rects.vertexCount++] = {
			{ rect.x, rect.y, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Rects.vertices[m_Rects.vertexCount++] = {
			{ rect.x + rect.w, rect.y, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Rects.vertices[m_Rects.vertexCount++] = {
			{ rect.x + rect.w, rect.y + rect.h, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Rects.vertices[m_Rects.vertexCount++] = {
			{ rect.x, rect.y + rect.h, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Rects.indices[m_Rects.indexCount++] = m_Rects.vertexCount - 4;
		m_Rects.indices[m_Rects.indexCount++] = m_Rects.vertexCount - 3;
		m_Rects.indices[m_Rects.indexCount++] = m_Rects.vertexCount - 2;
		m_Rects.indices[m_Rects.indexCount++] = m_Rects.vertexCount - 2;
		m_Rects.indices[m_Rects.indexCount++] = m_Rects.vertexCount - 1;
		m_Rects.indices[m_Rects.indexCount++] = m_Rects.vertexCount - 4;
	}

	void Context::DrawLine(const Point &p1, const Point &p2, float width, const Color &color)
	{
		if (m_Lines.vertexCount + 4 > BATCH_MAX_VERTICES) {
			FlushLines();
		}
		glm::vec2 dir = glm::normalize(glm::vec2(p2.x, p2.y) - glm::vec2(p1.x, p1.y));
		glm::vec2 normal = glm::vec2(-dir.y, dir.x) * width * 0.5f;
		m_Lines.vertices[m_Lines.vertexCount++] = {
			{ glm::vec2(p1.x, p1.y) + normal, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Lines.vertices[m_Lines.vertexCount++] = {
			{ glm::vec2(p2.x, p2.y) + normal, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Lines.vertices[m_Lines.vertexCount++] = {
			{ glm::vec2(p2.x, p2.y) - normal, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Lines.vertices[m_Lines.vertexCount++] = {
			{ glm::vec2(p1.x, p1.y) - normal, 0.0f, 1.0f },
			color,
			{ 0.f, 0.f }
		};
		m_Lines.indices[m_Lines.indexCount++] = m_Lines.vertexCount - 4;
		m_Lines.indices[m_Lines.indexCount++] = m_Lines.vertexCount - 3;
		m_Lines.indices[m_Lines.indexCount++] = m_Lines.vertexCount - 2;
		m_Lines.indices[m_Lines.indexCount++] = m_Lines.vertexCount - 2;
		m_Lines.indices[m_Lines.indexCount++] = m_Lines.vertexCount - 1;
		m_Lines.indices[m_Lines.indexCount++] = m_Lines.vertexCount - 4;
	}

	void Context::DrawGlyph(const Point &origin, const Color& color, const font::Glyph &glyph) {
		TextBatch *batch = nullptr;
		for (auto& b : m_Text) {
			if (b.atlasId == glyph.atlasId) {
				batch = &b;
				break;
			}
		}
		if (batch == nullptr) {
			batch = &m_Text.emplace_back();
			batch->atlasId = glyph.atlasId;
		}
		const font::AtlasPage& atlas = m_FontManager->GetAtlasPage(glyph.atlasId);
		if (batch->vertexCount + 4 > BATCH_MAX_VERTICES) {
			FlushText(m_Text[glyph.atlasId]);
		}
		float x0 = origin.x + glyph.bearingX;
		float y0 = origin.y - glyph.bearingY;
		float x1 = x0 + glyph.width;
		float y1 = y0 + glyph.height;
		batch->vertices[batch->vertexCount++] = {
			{ x0, y0, 0.0f, 1.0f },
			color,
			{ (float)glyph.atlasX / (float)atlas.width, (float)glyph.atlasY / (float)atlas.height }
		};
		batch->vertices[batch->vertexCount++] = {
			{ x1, y0, 0.0f, 1.0f },
			color,
			{ (float)(glyph.atlasX + glyph.width) / (float)atlas.width, (float)glyph.atlasY / (float)atlas.height }
		};
		batch->vertices[batch->vertexCount++] = {
			{ x1, y1, 0.0f, 1.0f },
			color,
			{ (float)(glyph.atlasX + glyph.width) / (float)atlas.width, (float)(glyph.atlasY + glyph.height) / (float)atlas.height }
		};
		batch->vertices[batch->vertexCount++] = {
			{ x0, y1, 0.0f, 1.0f },
			color,
			{ (float)glyph.atlasX / (float)atlas.width, (float)(glyph.atlasY + glyph.height) / (float)atlas.height }
		};
		batch->indices[batch->indexCount++] = batch->vertexCount - 4;
		batch->indices[batch->indexCount++] = batch->vertexCount - 3;
		batch->indices[batch->indexCount++] = batch->vertexCount - 2;
		batch->indices[batch->indexCount++] = batch->vertexCount - 2;
		batch->indices[batch->indexCount++] = batch->vertexCount - 1;
		batch->indices[batch->indexCount++] = batch->vertexCount - 4;
	}

	void Context::Flush() {
		PROFILE_SCOPE("GUI flush buffers");
		FlushTriangles();
		FlushRects();
		FlushLines();
		FlushText();
	}

	void Context::UseShader(GuiShader shader) {
		Flush();
		switch (shader) {
			case GuiShader::Flat:
				m_QuadFlatShader->Bind();
				break;
			case GuiShader::Text:
				m_TextShader->Bind();
				break;
			default:
				throw std::logic_error("Unknown shader");
		}
		m_CurrentShader = shader;
	}

	void Context::SetUniform(GuiShader shader, const std::string& name, const glm::mat4& value) {
		switch (shader) {
			case GuiShader::Flat:
				m_QuadFlatShader->Bind();
				m_QuadFlatShader->SetUniform(name, value);
				break;
			case GuiShader::Text:
				m_TextShader->Bind();
				m_TextShader->SetUniform(name, value);
				break;
			default:
				throw std::logic_error("Unknown shader");
		}
		UseShader(m_CurrentShader);
	}

	void Context::FlushTriangles() {
		PROFILE_SCOPE("GUI draw triangles");
		if (m_Triangles.indexCount == 0)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_Triangles.vertexCount * sizeof(Vertex), m_Triangles.vertices.data());
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Triangles.indexCount * sizeof(int16_t), m_Triangles.indices.data());
		glDrawElements(GL_TRIANGLES, m_Triangles.indexCount, GL_UNSIGNED_SHORT, nullptr);

		m_Triangles.vertexCount = 0;
		m_Triangles.indexCount = 0;
	}

	void Context::FlushRects() {
		PROFILE_SCOPE("GUI draw rects");
		if (m_Rects.indexCount == 0)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_Rects.vertexCount * sizeof(Vertex), m_Rects.vertices.data());
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Rects.indexCount * sizeof(int16_t), m_Rects.indices.data());
		glDrawElements(GL_TRIANGLES, m_Rects.indexCount, GL_UNSIGNED_SHORT, nullptr);

		m_Rects.vertexCount = 0;
		m_Rects.indexCount = 0;
	}

	void Context::FlushLines() {
		PROFILE_SCOPE("GUI draw lines");
		if (m_Lines.indexCount == 0)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_Lines.vertexCount * sizeof(Vertex), m_Lines.vertices.data());
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Lines.indexCount * sizeof(int16_t), m_Lines.indices.data());
		glDrawElements(GL_TRIANGLES, m_Lines.indexCount, GL_UNSIGNED_SHORT, nullptr);

		m_Lines.vertexCount = 0;
		m_Lines.indexCount = 0;
	}

	void Context::FlushText() {
		PROFILE_SCOPE("GUI draw text");
		for (auto& batch : m_Text) {
			FlushText(batch);
		}
	}

	void Context::FlushText(TextBatch& batch) {
		PROFILE_SCOPE_UNNAMED();
		if (batch.indexCount == 0)
			return;

		auto it = m_TextTextures.begin();
		for (; it != m_TextTextures.end(); it++) {
			if (it->atlasId == batch.atlasId) {
				if (m_FontManager->GetAtlasPage(batch.atlasId).version != it->atlasVersion)
					InvalidateAtlasTexture(*it);
				glBindTexture(GL_TEXTURE_2D, it->name);
				break;
			}
		}
		if (it == m_TextTextures.end())
			m_TextTextures.emplace_back(CreateAtlasTexture(batch.atlasId));

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batch.vertexCount * sizeof(Vertex), batch.vertices.data());
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batch.indexCount * sizeof(int16_t), batch.indices.data());
		glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_SHORT, nullptr);

		batch.vertexCount = 0;
		batch.indexCount = 0;
	}

	Context::AtlasTexture Context::CreateAtlasTexture(font::AtlasPageID id) {
		const font::AtlasPage& atlas = m_FontManager->GetAtlasPage(id);
		AtlasTexture tex;
		tex.atlasId = id;
		tex.atlasVersion = atlas.version;
		glGenTextures(1, &tex.name);
		glBindTexture(GL_TEXTURE_2D, tex.name);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlas.width, atlas.height, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return tex;
	}

	void Context::InvalidateAtlasTexture(AtlasTexture& tex) {
		const font::AtlasPage& atlas = m_FontManager->GetAtlasPage(tex.atlasId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas.width, atlas.height, GL_RED, GL_UNSIGNED_BYTE, atlas.data);
		tex.atlasVersion = atlas.version;
	}
}
}

