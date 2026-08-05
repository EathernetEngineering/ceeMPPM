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

#ifndef CEE_GUI_OBJECT_H_
#define CEE_GUI_OBJECT_H_

#include <cee/core/except.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace cee {
namespace gui {
	class GUIError : public core::Error {
	public:
		explicit GUIError(const std::string &object, const std::string &what)
		 : Error(what), m_ObjectName(object) {
		}
		explicit GUIError(const std::string &object, const char *what)
		 : Error(what), m_ObjectName(object) {
		}
		GUIError(const GUIError &other) = default;
		GUIError &operator=(const GUIError &other) = default;

		const std::string &GetObjectName() const noexcept { return m_ObjectName; }

	private:
		std::string m_ObjectName;
	};

	class ObjectError : public core::UsageError {
	public:
		explicit ObjectError(const std::string &object, const std::string &what)
		 : UsageError(what), m_ObjectName(object) {
		}
		explicit ObjectError(const std::string &object, const char *what)
		 : UsageError(what), m_ObjectName(object) {
		}
		ObjectError(const ObjectError &other) = default;
		ObjectError &operator=(const ObjectError &other) = default;

		const std::string &GetObjectName() const noexcept { return m_ObjectName; }

	private:
		std::string m_ObjectName;
	};

	namespace internal {
		int PrepareNode(void *ptr);
	}
	// struct Translation {
	// 	float x, y;
	// };

	struct Rect {
		float x, y, w, h;

		constexpr Rect() : x(0.f), y(0.f), w(0.f), h(0.f) {}
		constexpr Rect(float x, float y, float width, float height)
		 : x(x), y(y), w(width), h(height)
		{}
		constexpr Rect(const Rect& other)
		 : x(other.x), y(other.y), w(other.w), h(other.h)
		{}
		constexpr Rect(Rect&& other) noexcept
		 : x(other.x), y(other.y), w(other.w), h(other.h)
		{}
		constexpr Rect& operator=(const Rect& other) {
			x = other.x;
			y = other.y;
			w = other.w;
			h = other.h;
			return *this;
		}
		constexpr Rect& operator=(Rect&& other) noexcept {
			x = other.x;
			y = other.y;
			w = other.w;
			h = other.h;
			return *this;
		}
		~Rect() = default;
	};

	constexpr bool operator==(const Rect &lhs, const Rect &rhs) {
			constexpr float epsilon = 1e-6f;
			return (std::abs(lhs.x - rhs.x) <= epsilon) &&
				(std::abs(lhs.y - rhs.y) <= epsilon) &&
				(std::abs(lhs.w - rhs.w) <= epsilon) &&
				(std::abs(lhs.h - rhs.h) <= epsilon);
	}

	constexpr bool operator!=(const Rect &lhs, const Rect &rhs) {
		return !(lhs == rhs);
	}

	struct Point {
		float x, y;

		constexpr Point() : x(0.f), y(0.f) {}
		constexpr Point(float x, float y) : x(x), y(y) {}
		constexpr Point(const Point& other) : x(other.x), y(other.y) {}
		constexpr Point(Point&& other) noexcept : x(other.x), y(other.y) {}
		constexpr Point& operator=(const Point& other) {
			x = other.x;
			y = other.y;
			return *this;
		}
		constexpr Point& operator=(Point&& other) noexcept {
			x = other.x;
			y = other.y;
			return *this;
		}
		~Point() = default;

		glm::vec2 vec() const { return { x, y }; }
	};

	struct Size {
		union {
			struct { float w, h; };
			struct { float x, y; };
		};

		constexpr Size() : w(0.f), h(0.f) {}
		constexpr Size(float width, float height) : w(width), h(height) {}
		constexpr Size(const Size& other) : w(other.w), h(other.h) {}
		constexpr Size(Size&& other) noexcept : w(other.w), h(other.h) {}
		constexpr Size& operator=(const Size& other) {
			w = other.w;
			h = other.h;
			return *this;
		}
		constexpr Size& operator=(Size&& other) noexcept {
			w = other.w;
			h = other.h;
			return *this;
		}
		~Size() = default;
	};

	struct Color {
		float r, g, b, a;

		constexpr Color() : r(0.f), g(0.f), b(0.f), a(0.f) {}
		constexpr Color(float red, float green, float blue, float alpha = 1.f)
		 : r(red), g(green), b(blue), a(alpha)
		{}
		constexpr Color(const Color& other)
		 : r(other.r), g(other.g), b(other.b), a(other.a)
		{}
		constexpr Color(Color&& other) noexcept
		 : r(other.r), g(other.g), b(other.b), a(other.a)
		{}
		constexpr Color& operator=(const Color& other) {
			r = other.r;
			g = other.g;
			b = other.b;
			a = other.a;
			return *this;
		}
		constexpr Color& operator=(Color&& other) noexcept {
			r = other.r;
			g = other.g;
			b = other.b;
			a = other.a;
			return *this;
		}
		~Color() = default;
	};

	struct Constraints {
		float minWidth, minHeight;
		float maxWidth, maxHeight;
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
	private:
		class Impl;
		struct ImplDeleter {
			void operator()(Impl *p);
		};
		friend class Impl;

	protected:
		Object();
		virtual ~Object() = default;

	public:
		void Enable(bool enabled);
		void Show(bool show);
		bool IsEnabled() const;
		bool IsShown() const;

		void AddChild(Object *child);
		void RemoveChild(Object *child);
		bool HasChildren() const;

		void SetDebugName(const std::string &name) { m_DebugName = name; }
		const std::string& GetDebugName() const { return m_DebugName; }
	
	protected:
		void RenderChildren();

		virtual bool HasClip() const { return false; }
		virtual bool HasTransform() const { return false; }
		virtual Size Transform() const { return { 0.f, 0.f }; }
		virtual Rect Clip() const { return { 0.0f, 0.0f, 0.0f, 0.0f }; }

		virtual Size OnMeasure(const Constraints &c) { return { 0.f, 0.f }; (void)c; }
		virtual void OnArrange() {}
		virtual void OnRender() {}

		virtual bool CanHaveChildren() const { return false; }

		static Impl *GetImpl(Object *o) { return o->m_Impl.get(); }

	protected:
		std::string m_DebugName;

	protected:
		std::unique_ptr<Impl, ImplDeleter> m_Impl;

	public:
		friend int internal::PrepareNode(void *ptr);
		template<typename T, typename ...Args>
		requires std::derived_from<T, Object>
		friend std::unique_ptr<T> CreateNode(Args &&...args);
	};
}
}

#endif

