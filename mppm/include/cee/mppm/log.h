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

#ifndef CEE_LOG_H_
#define CEE_LOG_H_

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace cee {
class Log {
public:

	static void Init();
	static void Shutdown();

	static void AddLogger(std::shared_ptr<spdlog::logger> logger);
	static void RemoveLogger(std::shared_ptr<spdlog::logger> logger);
	static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }
	static std::shared_ptr<spdlog::logger> GetCoreLogger() { return s_CoreLogger; }

private:
	static void RemoveDeadChildren();

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
	static std::vector<std::weak_ptr<spdlog::logger>> s_Children;
};
}

// #define CEE_DEBUG(...)       ::cee::Log::GetLogger()->debug(__VA_ARGS__)
// #define CEE_TRACE(...)       ::cee::Log::GetLogger()->trace(__VA_ARGS__)
// #define CEE_INFO(...)        ::cee::Log::GetLogger()->info(__VA_ARGS__)
// #define CEE_WARN(...)        ::cee::Log::GetLogger()->warn(__VA_ARGS__)
// #define CEE_ERROR(...)       ::cee::Log::GetLogger()->error(__VA_ARGS__)
// #define CEE_CRITICAL(...)    ::cee::Log::GetLogger()->critical(__VA_ARGS__)

#define CEE_CORE_DEBUG(...)       ::cee::Log::GetCoreLogger()->debug(__VA_ARGS__)
#define CEE_CORE_TRACE(...)       ::cee::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CEE_CORE_INFO(...)        ::cee::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CEE_CORE_WARN(...)        ::cee::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CEE_CORE_ERROR(...)       ::cee::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CEE_CORE_CRITICAL(...)    ::cee::Log::GetCoreLogger()->critical(__VA_ARGS__)

#endif

