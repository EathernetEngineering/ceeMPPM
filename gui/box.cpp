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

#include <cee/gui/box.h>
#include <context.h>
#include <log.h>

#include <glad/gl.h>

namespace cee {
namespace gui {
	Size Box::OnMeasure(const Constraints &c) {
		if (m_ExplicitSize) {
			Constraints childConstraints = {
				.minWidth = c.minWidth,
				.minHeight = c.minHeight,
				.maxWidth = std::min(c.maxWidth, m_Desired.w),
				.maxHeight = std::min(c.maxHeight, m_Desired.h)
			};
			for (auto child : m_Children) {
				child->_Measure(childConstraints);
			}
			return m_Desired;
		}

		Size desired = { 0.f, 0.f };
		for (auto child : m_Children) {
			Size childDesired = child->_Measure(c);
			if (m_StackDirection == StackDirection::Horizontal) {
				desired.w += childDesired.w;
				desired.h = std::max(desired.h, childDesired.h);
			} else {
				desired.w = std::max(desired.w, childDesired.w);
				desired.h += childDesired.h;
			}
		}

		return desired;
	}

	void Box::OnArrange() {
		Rect childRect{};
		Size translation = { 0.f, 0.f };
		for (auto child : m_Children) {
			if (m_StackDirection == StackDirection::Horizontal) {
				childRect.x = translation.x;
				childRect.y = translation.y;
				childRect.w = child->_GetDesired().w;
				childRect.h = child->_GetDesired().h;
				translation.x += child->_GetDesired().w;
			} else {
				childRect.x = translation.x;
				childRect.y = translation.y;
				childRect.w = child->_GetDesired().w;
				childRect.h = child->_GetDesired().h;
				translation.y += child->_GetDesired().h;
			}
			child->_Arrange(childRect, m_AbsoluteRect);
		}
	}
	
	void Box::OnRender() {
		ctx->UseShader(Context::GuiShader::Flat);
		ctx->DrawRect(m_AbsoluteRect, m_Color);
	}
}
}

