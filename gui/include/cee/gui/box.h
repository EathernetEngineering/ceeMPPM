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

#ifndef CEE_GUI_BOX_H_
#define CEE_GUI_BOX_H_

#include <cee/gui/widget.h>
#include <glm/vec4.hpp>

#include <optional>

namespace cee {
namespace gui {
	class Box : public Widget {
	public:
		enum class StackDirection : uint8_t {
			Horizontal,
			Vertical
		};

	protected:
		Box();
		Box(float width, float height);
		Box(const Color &color);
		Box(float width, float height, const Color &color);

	public:
		virtual ~Box() = default;

		void Resize(float width, float height);

		Size GetSize() const;

		void SetStackDirection(StackDirection dir) {
			m_StackDirection = dir;
		}

		void SetColor(const Color& color) { m_Color = color; }

	protected:
		virtual bool HasClip() const override { return true; }
		virtual Rect Clip() const override;

		virtual Size OnMeasure(const Constraints &c) override;
		virtual void OnArrange() override;
		virtual void OnRender() override;

		virtual bool CanHaveChildren() const override { return true; }

	private:
		bool m_ExplicitSize;
		StackDirection m_StackDirection = StackDirection::Horizontal;
		std::optional<Color> m_Color;

	public:
		template<typename T, typename ...Args>
		requires std::derived_from<T, Object>
		friend std::unique_ptr<T> CreateNode(Args &&...args);
	};
}
}

#endif

