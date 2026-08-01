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

#ifndef CEE_GUI_TEXT_H_
#define CEE_GUI_TEXT_H_

#include <cee/gui/widget.h>

#include <cee/font/fonts.h>

#include <glm/vec4.hpp>

namespace cee {
namespace gui {
	class Text : public Widget {
	public:
		Text(const std::string &text, int size)
		 : m_Text(text), m_Size(size), m_Color(1.f, 1.f, 1.f, 1.f)
		{}
		Text(const std::string& text, int size, const Color& color)
		 : m_Text(text), m_Size(size), m_Color(color)
		{}
		virtual ~Text() = default;

		inline void SetText(const std::string &text) { m_Text = text; }
		inline void Resize(int size) { m_Size = size; }
		inline void SetColor(const Color& color) { m_Color = color; }

	protected:
		virtual bool HasClip() const override { return true; }
		virtual Rect Clip() const override { return m_AbsoluteRect; }

		virtual Size OnMeasure(const Constraints &c) override;
		virtual void OnRender() override;

	private:
		std::string m_Text;
		int m_Size;
		Color m_Color;
	};
}
}

#endif

