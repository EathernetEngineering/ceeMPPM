/*
 * CeeHealth
 * Copyright (C) 2025 2026 Chloe Eather
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

#include <cee/gui/object.h>
#include <context.h>

#include <algorithm>

namespace cee {
namespace gui {
	void Object::AddChild(Object *child) {
		m_Children.push_back(child);
		child->ctx = this->ctx;
	}

	void Object::RemoveChild(Object *child) {
		auto it = std::find(m_Children.begin(), m_Children.end(), child);
		if (it != m_Children.end()) {
			m_Children.erase(it);
		}
	}

	Size Object::_Measure(const Constraints &c) {
		if (!m_Enabled)
			return { 0.f, 0.f };

		m_Desired = OnMeasure(c);
		return m_Desired;
	}

	void Object::_Arrange(const Rect &rect, const Rect &parentAbsRect) {
		if (!m_Enabled)
			return;

		m_Rect = rect;
		m_AbsoluteRect = {
			parentAbsRect.x + rect.x,
			parentAbsRect.y + rect.y,
			rect.w,
			rect.h
		};
		if (m_Hidden)
			return;
		OnArrange();
	}

	void Object::_RenderTree() {
		if (m_Hidden)
			return;
		if (HasClip()) {
			auto clip = Clip();
			ctx->PushClip({ clip.x, clip.y, clip.w, clip.h });
		}
		if (HasTransform()) {
			ctx->PushTransform(Transform());
		}
		OnRender();
		RenderChildren();

		if (HasTransform()) {
			ctx->PopTransform();
		}
		if (HasClip()) {
			ctx->PopClip();
		}
	}

	void Object::RenderChildren() {
		for (const auto& child : m_Children) {
			child->_RenderTree();
		}
	}
}
}

