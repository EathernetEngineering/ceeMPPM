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

#ifndef CEE_HAL_LOG_H_
#define CEE_HAL_LOG_H_

#if defined(__cplusplus)
extern "C" {
#endif

void HALDebugMessage(const char *fmt, ...);
void HALTraceMessage(const char *fmt, ...);
void HALInfoMessage(const char *fmt, ...);
void HALWarnMessage(const char *fmt, ...);
void HALErrorMessage(const char *fmt, ...);
void HALCriticalMessage(const char *fmt, ...);

#if defined(__cplusplus)
}

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace cee {
namespace hal {
	class Log {
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
}
}

#define CEE_DEBUG(...)       ::cee::hal::Log::GetLogger()->debug(__VA_ARGS__)
#define CEE_TRACE(...)       ::cee::hal::Log::GetLogger()->trace(__VA_ARGS__)
#define CEE_INFO(...)        ::cee::hal::Log::GetLogger()->info(__VA_ARGS__)
#define CEE_WARN(...)        ::cee::hal::Log::GetLogger()->warn(__VA_ARGS__)
#define CEE_ERROR(...)       ::cee::hal::Log::GetLogger()->error(__VA_ARGS__)
#define CEE_CRITICAL(...)    ::cee::hal::Log::GetLogger()->critical(__VA_ARGS__)
#else
#define CEE_DEBUG(...)       HALDebugMessage(__VA_ARGS__)
#define CEE_TRACE(...)       HALTraceMessage(__VA_ARGS__)
#define CEE_INFO(...)        HALInfoMessage(__VA_ARGS__)
#define CEE_WARN(...)        HALWarnMessage(__VA_ARGS__)
#define CEE_ERROR(...)       HALErrorMessage(__VA_ARGS__)
#define CEE_CRITICAL(...)    HALCriticalMessage(__VA_ARGS__)
#endif

#endif

