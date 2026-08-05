/*
 * ceeGUI
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

#include <object_impl.h>
#include <context.h>

#include <cee/core/log.h>

#include <algorithm>

namespace cee {
namespace gui {
	Object::Impl::Impl(Object &obj) : obj(obj) {
	}

	Size Object::Impl::Measure(const Constraints &c) {
		if (!m_Enabled)
			return { 0.f, 0.f };

		m_Desired = obj.OnMeasure(c);
		return m_Desired;
	}

	void Object::Impl::Arrange(const Rect &rect, const Rect &parentAbsRect) {
		if (!m_Enabled)
			return;

		m_Rect = rect;
		m_AbsoluteRect = {
			parentAbsRect.x + rect.x,
			parentAbsRect.y + rect.y,
			rect.w,
			rect.h
		};
		obj.OnArrange();
	}

	void Object::Impl::RenderTree() {
		if (!m_Enabled)
			return;

		if (m_ShouldShow) {
			if (obj.HasClip()) {
				auto clip = obj.Clip();
				ctx->PushClip({ clip.x, clip.y, clip.w, clip.h });
			}
			if (obj.HasTransform()) {
				ctx->PushTransform(obj.Transform());
			}
			obj.OnRender();
		}

		obj.RenderChildren();

		if (m_ShouldShow) {
			if (obj.HasTransform()) {
				ctx->PopTransform();
			}
			if (obj.HasClip()) {
				ctx->PopClip();
			}
		}
	}


	void Object::ImplDeleter::operator()(Impl *p) {
		delete p;
	}

	Object::Object() {
		m_Impl = std::unique_ptr<Impl, ImplDeleter>(new Impl(*this));
	}

	void Object::Enable(bool enabled) {
		m_Impl->m_Enabled = enabled;
	}

	void Object::Show(bool show) {
		m_Impl->m_ShouldShow = show;
	}

	bool Object::IsEnabled() const {
		return m_Impl->m_Enabled;
	}

	bool Object::IsShown() const {
		return m_Impl->m_ShouldShow;
	}

	bool Object::HasChildren() const {
		return m_Impl->m_Children.size() > 0;
	}

	void Object::AddChild(Object *child) {
		if (child == this)
			throw ObjectError(m_DebugName, "object cannot be it's own parent");
		if (CanHaveChildren() == false)
			throw ObjectError(m_DebugName, "object cannot have children");

		m_Impl->m_Children.push_back(child);
	}

	void Object::RemoveChild(Object *child) {
		if (CanHaveChildren() == false) {
			throw ObjectError(m_DebugName, "Object cannot have children");
		}
		auto it = std::find(m_Impl->m_Children.begin(), m_Impl->m_Children.end(), child);
		if (it != m_Impl->m_Children.end()) {
			m_Impl->m_Children.erase(it);
		}
	}

	void Object::RenderChildren() {
		for (const auto& child : m_Impl->m_Children) {
			child->m_Impl->RenderTree();
		}
	}
}
}

