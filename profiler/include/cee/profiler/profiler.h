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

#ifndef CEE_PROFILER_H_
#define CEE_PROFILER_H_

#if defined(__clang__) || defined(__GNUC__)
#define PRETTY_SIGNATURE __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define PRETTY_SIGNATURE __FUNCSIG__
#else
#define PRETTY_SIGNATURE __func__
#endif

#ifdef CEE_PROFILER_ENABLE
#include <Tracy.hpp>
#define PROFILE_SCOPE(name) ZoneScopedN(name)
#define PROFILE_SCOPE_UNNAMED() ZoneScoped
#define PROFILE_FUNCTION() ZoneScopedN(PRETTY_SIGNATURE)
#define PROFILE_FUNCTION_NAMED(name) ZoneScopedN(PRETTY_SIGNATURE ": " name)
#define PROFILER_FRAME_MARK() FrameMark
#else
#define PROFILE_SCOPE(name)
#define PROFILE_SCOPE_UNNAMED()
#define PROFILE_FUNCTION()
#define PROFILE_FUNCTION_NAMED(name)
#define PROFILER_FRAME_MARK()
#endif

#endif

