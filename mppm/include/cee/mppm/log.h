/*
 * ceeMPPM
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

#ifndef CEE_MPPM_LOG_H_
#define CEE_MPPM_LOG_H_

#include <cee/mppm/mppm.h>

#define CEE_CORE_DEBUG(...)       ::cee::MPPM::GetLogger()->debug(__VA_ARGS__)
#define CEE_CORE_TRACE(...)       ::cee::MPPM::GetLogger()->trace(__VA_ARGS__)
#define CEE_CORE_INFO(...)        ::cee::MPPM::GetLogger()->info(__VA_ARGS__)
#define CEE_CORE_WARN(...)        ::cee::MPPM::GetLogger()->warn(__VA_ARGS__)
#define CEE_CORE_ERROR(...)       ::cee::MPPM::GetLogger()->error(__VA_ARGS__)
#define CEE_CORE_CRITICAL(...)    ::cee::MPPM::GetLogger()->critical(__VA_ARGS__)

#endif

