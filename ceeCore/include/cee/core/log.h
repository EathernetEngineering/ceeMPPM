/*
 * ceeCore
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

#ifndef CEE_CORE_LOG_H_
#define CEE_CORE_LOG_H_

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace cee {
	using Logger = std::shared_ptr<spdlog::logger>;

	class Log {
	public:
		Log(const std::string &name,
				const std::string &logFile,
				spdlog::level::level_enum level = spdlog::level::info);

		Logger CreateChild(const std::string &name);

		Logger &GetLogger() { return m_CoreLogger; }
		
	private:
		std::string m_LogFile;
		spdlog::level::level_enum m_LogLevel;
		std::vector<spdlog::sink_ptr> m_Sinks;
		Logger m_CoreLogger;
		std::vector<std::weak_ptr<spdlog::logger>> m_Children;
	};
}

#endif

