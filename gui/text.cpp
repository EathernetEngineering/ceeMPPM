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

#include <cee/gui/text.h>
#include <object_impl.h>
#include <cee/gui/util.h>

#include <cee/font/fonts.h>

#include <glad/gl.h>

#include <algorithm>

namespace cee {
namespace gui {
	Rect Text::Clip() const {
		return m_Impl->m_AbsoluteRect;
	}

	Size Text::OnMeasure(const Constraints &c) {
		Size desired = { 0.f, 0.f };

		int penX = 0, lines = 1;
		for (size_t offset = 0; offset < m_Text.size();) {
			Utf8Result result = decodeUtf8(m_Text.data() + offset, m_Text.size() - offset);
			if (result.length == 0)
				break;

			const font::Glyph &glyph = m_Impl->ctx->GetDefaultFont()->TryLoadGlyph(result.codepoint, m_Size);
			if (glyph.width + penX > c.maxWidth) {
				penX = 0;
				desired.w = c.maxWidth;
				lines++;
			}
			penX += glyph.advance;

			offset += result.length;
		}

		desired.w = std::max((float)penX, desired.w);
		desired.h = lines * m_Impl->ctx->GetDefaultFont()->GetLineHeight(m_Size);

		return desired;
	}
	
	void Text::OnRender() {
		m_Impl->ctx->UseShader(Context::GuiShader::Texture);
		font::Glyph glyph;
		int penX = 0, penY = m_Impl->ctx->GetDefaultFont()->GetAscent(m_Size);
		Point glyphOrigin{};
		for (size_t offset = 0; offset < m_Text.size();) {
			Utf8Result result = decodeUtf8(m_Text.data() + offset, m_Text.size() - offset);
			if (result.length == 0)
				break;

			glyph = m_Impl->ctx->GetDefaultFont()->TryLoadGlyph(result.codepoint, m_Size);
			if (glyph.width + penX > m_Impl->m_Rect.w) {
				penX = 0;
				penY += m_Impl->ctx->GetDefaultFont()->GetLineHeight(m_Size);
			}
			glyphOrigin.x = m_Impl->m_AbsoluteRect.x + penX;
			glyphOrigin.y = m_Impl->m_AbsoluteRect.y + penY;
			if (glyph.width > 0)
				m_Impl->ctx->DrawGlyph(glyphOrigin, m_Color, glyph);
			penX += glyph.advance;
			penY += 0;

			offset += result.length;
		}
	}
}
}

