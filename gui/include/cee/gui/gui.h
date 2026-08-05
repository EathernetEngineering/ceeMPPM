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

#ifndef CEE_GUI_GUI_H_
#define CEE_GUI_GUI_H_

#include <cee/gui/widget.h>

#include <cee/font/fonts.h>

#include <cee/core/log.h>

#include <memory>

namespace cee {
namespace gui {
	namespace internal {
		int PrepareNode(void *ptr);
	}

	int Init(Logger logger = nullptr);
	void Shutdown();

	void AddFont(std::shared_ptr<font::Font> font);

	void SetRootNode(Widget *node);

	template<typename T, typename... Args>
	requires std::derived_from<T, Object>
	std::unique_ptr<T> CreateNode(Args &&...args) {
		std::unique_ptr<T> node(new T(std::forward<Args>(args)...));
		int result = internal::PrepareNode(dynamic_cast<Object *>(node.get())->m_Impl.get());
		if (result) {
			throw GUIError(node->GetDebugName(), "Failed to prepare node {}");
		}
		return node;
	}

	int BeginFrame(const Size &viewport);
	int Render(const Size &viewport);
	void EndFrame();
	inline int HandleEvents() { return 0; }
}
}

#endif

