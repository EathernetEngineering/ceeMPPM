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

#include <cee/gui/box.h>
#include <object_impl.h>

#include <glad/gl.h>

namespace cee {
namespace gui {
	Box::Box()
	 : m_ExplicitSize(false), m_Color(std::nullopt) {
	}

	Box::Box(float width, float height)
	 : m_ExplicitSize(true), m_Color(std::nullopt) {
		m_Impl->m_Desired = { width, height };
	}

	Box::Box(const Color &color)
	 : m_ExplicitSize(false), m_Color(color) {
	}

	Box::Box(float width, float height, const Color &color)
	 : m_ExplicitSize(true), m_Color(color) {
		m_Impl->m_Desired = { width, height };
	}

	void Box::Resize(float width, float height) {
		if (width == 0.f && height == 0.f) {
			m_ExplicitSize = false;
			return;
		}
		m_ExplicitSize = true;
		m_Impl->m_Desired = { width, height };
	}

	Size Box::GetSize() const {
		return m_Impl->m_Desired;
	}

	Rect Box::Clip() const {
		return m_Impl->m_AbsoluteRect;
	}

	Size Box::OnMeasure(const Constraints &c) {
		if (m_ExplicitSize) {
			Constraints childConstraints = {
				.minWidth = c.minWidth,
				.minHeight = c.minHeight,
				.maxWidth = std::min(c.maxWidth, m_Impl->m_Desired.w),
				.maxHeight = std::min(c.maxHeight, m_Impl->m_Desired.h)
			};
			for (auto child : m_Impl->m_Children) {
				GetImpl(child)->Measure(childConstraints);
			}
			return m_Impl->m_Desired;
		}

		Size desired = { 0.f, 0.f };
		for (auto child : m_Impl->m_Children) {
			Size childDesired = GetImpl(child)->Measure(c);
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
		for (auto child : m_Impl->m_Children) {
			if (m_StackDirection == StackDirection::Horizontal) {
				childRect.x = translation.x;
				childRect.y = translation.y;
				childRect.w = GetImpl(child)->GetDesired().w;
				childRect.h = GetImpl(child)->GetDesired().h;
				translation.x += GetImpl(child)->GetDesired().w;
			} else {
				childRect.x = translation.x;
				childRect.y = translation.y;
				childRect.w = GetImpl(child)->GetDesired().w;
				childRect.h = GetImpl(child)->GetDesired().h;
				translation.y += GetImpl(child)->GetDesired().h;
			}
			GetImpl(child)->Arrange(childRect, m_Impl->m_AbsoluteRect);
		}
	}
	
	void Box::OnRender() {
		if (m_Color.has_value()) {
			m_Impl->ctx->UseShader(Context::GuiShader::Flat);
			m_Impl->ctx->DrawRect(m_Impl->m_AbsoluteRect, m_Color.value());
		}
	}
}
}

