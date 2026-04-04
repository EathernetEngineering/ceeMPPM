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

#ifndef CEE_GUI_BOX_H_
#define CEE_GUI_BOX_H_

#include <cee/gui/widget.h>
#include <glm/vec4.hpp>

namespace cee {
namespace gui {
	class Box : public Widget {
	public:
		enum class StackDirection : uint8_t {
			Horizontal,
			Vertical
		};

	public:
		Box()
		 : m_ExplicitSize(false), m_Color(0.f, 0.f, 0.f, 0.f)
		{}
		Box(float width, float height)
		 : m_ExplicitSize(true), m_Color(0.f, 0.f, 0.f, 0.f)
		{
			m_Desired = { width, height };
		}
		Box(const Color& color)
		 : m_ExplicitSize(false), m_Color(color)
		{}
		Box(float width, float height, const Color& color)
		 : m_ExplicitSize(true), m_Color(color)
		{
			m_Desired = { width, height };
		}
		virtual ~Box() = default;

		inline void Resize(float width, float height) {
			if (width == 0.f && height == 0.f) {
				m_ExplicitSize = false;
				return;
			}
			m_ExplicitSize = true;
			m_Desired = { width, height };
		}
		inline Size GetSize() const { return m_Desired; }

		inline void SetStackDirection(StackDirection dir) {
			m_StackDirection = dir;
		}

		inline void SetColor(const Color& color) { m_Color = color; }

	protected:
		virtual bool HasClip() const override { return true; }
		virtual Rect Clip() const override { return m_AbsoluteRect; }

		virtual Size OnMeasure(const Constraints &c) override;
		virtual void OnArrange() override;
		virtual void OnRender() override;

	private:
		std::vector<Size> MeasureChildren(const Constraints &c);

	private:
		bool m_ExplicitSize;
		StackDirection m_StackDirection = StackDirection::Horizontal;
		Color m_Color;
	};
}
}

#endif

