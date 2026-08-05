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

#ifndef CEE_GUI_OBJECT_IMPL_H_
#define CEE_GUI_OBJECT_IMPL_H_

#include <cee/gui/object.h>

#include <context.h>

namespace cee {
namespace gui {
	class Object::Impl {
	public:
		Impl(Object &obj);

		Size Measure(const Constraints &c);
		void Arrange(const Rect &rect, const Rect &parentAbsRect);
		void RenderTree();

		Size GetDesired() const { return m_Desired; }
		Rect GetRect() const { return m_Rect; }
		Rect GetAbsoluteRect() const { return m_AbsoluteRect; }

	public:
		std::shared_ptr<Context> ctx;
		bool m_Enabled = true;
		bool m_ShouldShow = false;
		bool m_LayoutStale = true;
		std::vector<Object *> m_Children;

		Size m_Desired{};
		Rect m_Rect{};
		Rect m_AbsoluteRect{};

	private:
		Object &obj;
	};
}
}

#endif

