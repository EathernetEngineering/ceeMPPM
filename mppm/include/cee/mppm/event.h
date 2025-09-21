/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
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

#ifndef CEE_EVENT_H_
#define CEE_EVENT_H_

#include <cstdint>

namespace cee {
enum EventClass {
	EVENT_CLASS_APPLICATION = 1 << 0,
	EVENT_CLASS_KEYBOARD    = 1 << 1,
	EVENT_CLASS_MOUSE       = 1 << 2
};

enum class EventType : uint8_t {
	EVENT_TYPE_APPLICATION_TICK = 0, EVENT_TYPE_APPLICATION_EXIT, EVENT_TYPE_PAGE_FLIP,
	EVENT_TYPE_KEYBOARD_KEY_DOWN, EVENT_TYPE_KEYBOARD_KEY_UP, EVENT_TYPE_KEYBOARD_KEY_REPEAT,
	EVENT_TYPE_MOUSE_MOVE, EVENT_TYPE_MOUSE_BUTTON_DOWN, EVENT_TYPE_MOUSE_BUTTON_UP, EVENT_TYPE_MOUSE_SCROLL
};

class Event {
public:
	virtual EventClass GetClass() = 0;
	virtual EventType GetType() = 0;
};

class ApplicationTickEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_APPLICATION; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_APPLICATION_TICK; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_APPLICATION_TICK; }

public:
	ApplicationTickEvent(float fdt)
	 : m_fDT(fdt)
	{ }

	virtual float GetDeltaTime() const { return m_fDT; }

protected:
	float m_fDT;
};

class ApplicationExitEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_APPLICATION; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_APPLICATION_EXIT; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_APPLICATION_EXIT; }
};

class ApplicationPageFlip : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_APPLICATION; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_PAGE_FLIP; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_PAGE_FLIP; }
};

class KeyDownEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_KEYBOARD; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_KEYBOARD_KEY_DOWN; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_KEYBOARD_KEY_DOWN; }

public:
	KeyDownEvent(uint32_t uKeycode)
	 : m_uKeycode(uKeycode)
	{ }

	virtual uint32_t GetKeycode() const { return m_uKeycode; }

protected:
	uint32_t m_uKeycode;
};

class KeyUpEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_KEYBOARD; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_KEYBOARD_KEY_UP; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_KEYBOARD_KEY_UP; }

public:
	KeyUpEvent(uint32_t uKeycode)
	 : m_uKeycode(uKeycode)
	{ }

	virtual uint32_t GetKeycode() const { return m_uKeycode; }

protected:
	uint32_t m_uKeycode;
};

class KeyRepeatEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_KEYBOARD; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_KEYBOARD_KEY_REPEAT; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_KEYBOARD_KEY_REPEAT; }

public:
	KeyRepeatEvent(uint32_t uKeycode)
	 : m_uKeycode(uKeycode)
	{ }

	virtual uint32_t GetKeycode() const { return m_uKeycode; }

protected:
	uint32_t m_uKeycode;
};

class MouseMoveEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_MOUSE; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_MOUSE_MOVE; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_MOUSE_MOVE; }

public:
	MouseMoveEvent(double nX, double nY)
	 : m_nX(nX), m_nY(nY)
	{ }

	virtual double GetX() const { return m_nX; }
	virtual double GetY() const { return m_nY; }

protected:
	double m_nX, m_nY;
};

class ButtonDownEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_MOUSE; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_MOUSE_BUTTON_DOWN; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_MOUSE_BUTTON_DOWN; }

public:
	ButtonDownEvent(uint32_t uMousecode)
	 : m_uMousecode(uMousecode)
	{ }

	virtual uint32_t GetMousecode() const { return m_uMousecode; }

protected:
	uint32_t m_uMousecode;
};

class ButtonUpEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_MOUSE; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_MOUSE_BUTTON_UP; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_MOUSE_BUTTON_UP; }

public:
	ButtonUpEvent(uint32_t uMousecode)
	 : m_uMousecode(uMousecode)
	{ }

	virtual uint32_t GetMousecode() const { return m_uMousecode; }

protected:
	uint32_t m_uMousecode;
};

class ScrollEvent : public Event {
public:
	virtual EventClass GetClass() { return EVENT_CLASS_MOUSE; };
	virtual EventType GetType() { return EventType::EVENT_TYPE_MOUSE_SCROLL; };

	static EventType GetStaticType() { return EventType::EVENT_TYPE_MOUSE_SCROLL; }

public:
	ScrollEvent(double nX, double nY)
	 : m_nX(nX), m_nY(nY)
	{ }

	virtual double GetX() const { return m_nX; }
	virtual double GetY() const { return m_nY; }

protected:
	double m_nX, m_nY;
};

class EventDispatcher {
public:
	EventDispatcher(Event &e)
	 : m_Event(e)
	{ }

	template<typename T, typename F>
	bool Dispatch(const F &func) {
		if (m_Event.GetType() == T::GetStaticType()) {
			func(dynamic_cast<T&>(m_Event));
			return true;
		}
		return false;
	}

private:
	Event &m_Event;
};
}

#endif
