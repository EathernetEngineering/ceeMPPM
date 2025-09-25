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

#ifndef CEE_HAL_LOG_PUBLIC_H_
#define CEE_HAL_LOG_PUBLIC_H_

#ifdef __cplusplus
extern "C" {
#endif
enum LogLevel {
	CEE_LOG_LEVEL_DEBUG = 0,
	CEE_LOG_LEVEL_TRACE = 1,
	CEE_LOG_LEVEL_INFO = 2,
	CEE_LOG_LEVEL_WARN = 3,
	CEE_LOG_LEVEL_ERROR = 4,
	CEE_LOG_LEVEL_CRITICAL = 5,
	CEE_LOG_ENUM_MAX = CEE_LOG_LEVEL_CRITICAL
};
void HALLogInit(void);
void HALLogShutdown(void);
void HALLogSetLevel(LogLevel level);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <spdlog/spdlog.h>

#include <memory>

namespace cee {
namespace hal {
	std::shared_ptr<spdlog::logger> GetLogger();
}
}
#endif

#endif

