/*
 * ceeGUI
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

#ifndef CEE_GUI_CONTEXT_H_
#define CEE_GUI_CONTEXT_H_

#include <cee/gui/object.h>
#include <shaders.h>

#include <cee/core/except.h>

#include <cee/font/fonts.h>

#include <cee/platform/gfx.h>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <stack>
#include <unordered_map>

namespace cee {
namespace gui {
	constexpr int BATCH_MAX_VERTICES = 16384;
	constexpr int BATCH_MAX_INDICES = 24576;

	struct Vertex {
		glm::vec4 position;
		Color color;
		glm::vec2 uv;
	};

	class Context {
	public:
		enum class GuiShader {
			Flat,
			Texture
		};

	public:
		Context(Logger logger = nullptr);
		~Context();

		void SetViewport(const Size &viewport);

		void PushClip(const Rect &clip);
		void PopClip();
		void PushTransform(const Size &transform);
		void PopTransform();

		void DrawTriangle(const Point &a, const Point &b, const Point &c, const Color &color);
		void DrawRect(const Rect &rect, const Color &color);
		void DrawLine(const Point &p1, const Point &p2, float width, const Color &color);
		void DrawPolyLine(std::span<const Point> points, float width, const Color &color);
		void DrawPolyLine(const std::vector<Point> &points, float width, const Color &color) {
			DrawPolyLine(std::span(points.cbegin(), points.cend()), width, color);
		}
		void DrawGlyph(const Point &origin, const Color& color, const font::Glyph &glyph);
		void DrawText(const std::string &text, const Point &position, const Color &color);
		void Flush();

		void UseShader(GuiShader shader);
		void SetUniform(GuiShader shader, const std::string &name, const glm::mat4 &value);

		const glm::mat4& GetProjection() const { return m_Projection; }
		font::Font *GetDefaultFont() const { return m_Fonts[0].get(); };

		template<typename T>
		void Log(spdlog::level::level_enum level, const T &msg) {
			if (m_Logger)
				m_Logger->log(level, msg);
		}

		template<typename ...Args>
		void Log(spdlog::level::level_enum level, spdlog::format_string_t<Args...> fmt, Args &&...args) {
			if (m_Logger)
				m_Logger->log(level, fmt, std::forward<Args>(args)...);
		}

	private:
		struct TriangleBatch {
			std::array<Vertex, BATCH_MAX_VERTICES> vertices;
			std::array<int16_t, BATCH_MAX_VERTICES> indices;
			int vertexCount = 0;
			int indexCount = 0;
		};

		struct RectBatch {
			std::array<Vertex, BATCH_MAX_VERTICES> vertices;
			std::array<int16_t, (BATCH_MAX_VERTICES*6)/4> indices;
			int vertexCount = 0;
			int indexCount = 0;
		};

		struct LineBatch {
			std::array<Vertex, BATCH_MAX_VERTICES> vertices;
			std::array<int16_t, (BATCH_MAX_VERTICES*6)/4> indices;
			int vertexCount = 0;
			int indexCount = 0;
		};

		struct TextBatch {
			font::AtlasPageID atlasId;
			std::array<Vertex, BATCH_MAX_VERTICES> vertices;
			std::array<int16_t, (BATCH_MAX_VERTICES*6)/4> indices;
			int vertexCount = 0;
			int indexCount = 0;
		};
		struct AtlasTexture {
			int atlasId;
			int atlasVersion;
			GLuint name;
		};

	private:
		GLint GetUniformLocation(const std::string& name);
		void FlushTriangles();
		void FlushRects();
		void FlushLines();
		void FlushText();
		void FlushText(TextBatch& batch);
		AtlasTexture CreateAtlasTexture(font::AtlasPageID id);
		void InvalidateAtlasTexture(AtlasTexture& tex);

	private:
		Logger m_Logger;
		std::unique_ptr<Shader> m_QuadFlatShader;
		std::unique_ptr<Shader> m_TextShader;
		std::unique_ptr<font::FontManager> m_FontManager;
		std::vector<std::shared_ptr<font::Font>> m_Fonts;
		uint32_t m_FontTexture;
		uint32_t m_VBO, m_EBO;
		GuiShader m_CurrentShader;
		Size m_Viewport;
		glm::mat4 m_Projection;
		std::unordered_map<std::string, GLint> m_UniformLocations;
		TriangleBatch m_Triangles;
		RectBatch m_Rects;
		LineBatch m_Lines;
		std::vector<TextBatch> m_Text;
		std::vector<AtlasTexture> m_TextTextures;
		std::stack<Size> m_TransformStack;
		std::stack<Rect> m_ClipStack;
	};
}
}

#endif

