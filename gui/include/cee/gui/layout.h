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

#ifndef CEE_GUI_LAYOUT_H_
#define CEE_GUI_LAYOUT_H_

#include "gui.h"
#include "types.h"
#include "widget.h"

#include <algorithm>
#include <type_traits>

namespace cee {
namespace GUI {
template<typename Enum> class Flags;

class Flag {
public:
	constexpr inline Flag(int val) noexcept : i(val) {}
	constexpr inline operator int() const noexcept { return i; }
private:
	int i;
};

class IncompatibleFlag {
public:
	constexpr inline explicit IncompatibleFlag(int i) noexcept;
	constexpr inline operator int() const noexcept { return i; }
private:
	int i;
};

namespace ceePrivate {
template<typename T> struct IsFlags : std::false_type {};
template<typename E> struct IsFlags<Flags<E>> : std::true_type {};


template<typename Enum>
class FlagsStorage {
	static_assert(sizeof(Enum) <= sizeof(int64_t), 
	              "Only enumerations 64 bits or less are supported");
	static_assert((std::is_enum<Enum>::value),
	              "Flags is only usable on enumeration types.");

	static constexpr size_t IntegerSize = (std::max)(sizeof(Enum), sizeof(int));
	using Integers = Enum;
};
}

enum AlignmentFlag {
	AlignLeft     = 0x0001,
	AlignRight    = 0x0002,
	AlignHCenter  = 0x0004,
	AlignJustify  = 0x0008,
	AlignTop      = 0x0020,
	AlignBottom   = 0x0040,
	AlignVCenter  = 0x0080,
	AlignBaseline = 0x0100,
	AlignCenter   = AlignVCenter | AlignHCenter
};
using Alignment = flags;
}

class Layout : public Widget {
	Layout() = default;
	~Layout() = default;
};
}

#endif

