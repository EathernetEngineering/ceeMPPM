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

#ifndef CEE_GUI_OBJECT_H_
#define CEE_GUI_OBJECT_H_

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <any>
#include <functional>
#include <set>
#include <string>
#include <utility>

#include <cstdint>

namespace cee {
namespace gui {
	class Context;

	enum class Signal : int64_t {
		None = 0,
		Activate = 1,
		Destroy = 2,
		Hide = 3,
		Show = 4,
		Clicked = 5
	};

	template<typename... Args>
	struct SignalHandler {
		Signal sig;
		std::function<void(Args...)> f;

		void invoke(Args&&... args);

		void operator()(Args... args) {
			invoke(std::forward<Args>(args)...);
		}

		bool operator==(const SignalHandler<Args...>& other) {
			return (this->sig == other.sig) &&
			(std::hash<SignalHandler<Args...>>(*this) == std::hash<SignalHandler<Args...>>(other));
		}
	};

	struct Translation {
		float x, y;
	};

	struct Rect {
		float x, y, w, h;
	};

	struct Point {
		float x, y;
	};

	struct Size {
		float w, h;
	};

	struct Constraints {
		float minWidth, minHeight;
		float maxWidth, maxHeight;
	};

	struct Color {
		float r, g, b, a;
	};

	constexpr inline Color HexToColor(uint32_t hex) {
		return {
			((hex >> 24) & 0xFF) / 255.f,
			((hex >> 16) & 0xFF) / 255.f,
			((hex >> 8) & 0xFF) / 255.f,
			(hex & 0xFF) / 255.f
		};
	}

	template<typename T>
	requires std::is_arithmetic_v<T>
	constexpr inline T Clamp(T val, T min, T max) {
		if (val > max)
			return max;
		if (val < min)
			return min;
		return val;
	}

	constexpr inline Size ClampSize(Size s, const Constraints &c) {
		return { Clamp(s.w, c.minWidth, c.maxWidth), Clamp(s.h, c.minHeight, c.maxHeight) };
	}

	class Object {
	public:
		Object() = default;
		virtual ~Object() = default;

		template <typename... Args>
		int64_t SignalConnect(const Signal sig, std::function<void(Args...)> f);
		void SignalDisconnect(int64_t handlerId);
		template <typename... Args>
		void SignalDisconnectByFunc(const Signal sig, std::function<void(Args...)> f);

		void Enable(bool enabled) {
			m_Enabled = !enabled;
		}

		void Show(bool show) {
			m_Hidden = !show;
		}

		bool IsEnabled() const { return m_Enabled; }
		bool IsHidden() const { return m_Hidden; }

		void AddChild(Object *child);
		void RemoveChild(Object *child);

		void SetDebugName(const std::string &name) { m_DebugName = name; }
		const std::string& GetDebugName() const { return m_DebugName; }

	public:
		Size _Measure(const Constraints &c);
		void _Arrange(const Rect &rect, const Rect &parentAbsRect);
		void _RenderTree();

		Size _GetDesired() const { return m_Desired; }
		Rect _GetRect() const { return m_Rect; }
		Rect _GetAbsoluteRect() const { return m_AbsoluteRect; }
	
	protected:
		void RenderChildren();

		virtual bool HasClip() const { return false; }
		virtual bool HasTransform() const { return false; }
		virtual glm::mat4 Transform() const { return glm::identity<glm::mat4>(); }
		virtual Rect Clip() const { return { 0.0f, 0.0f, 0.0f, 0.0f }; }

		virtual Size OnMeasure(const Constraints &c) { return { 0.f, 0.f }; }
		virtual void OnArrange() {}
		virtual void OnRender() {}

	protected:
		Context *ctx = nullptr;
		bool m_Enabled = true;
		bool m_Hidden = false;
		std::vector<Object *> m_Children;

		Size m_Desired{};
		Rect m_Rect{};
		Rect m_AbsoluteRect{};

		std::string m_DebugName;

	private:
		std::set<std::any> m_SignalCallbacks;
	};
}
}

#endif

