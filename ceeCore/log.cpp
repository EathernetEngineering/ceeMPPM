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

#include <cee/core/log.h>
#include <cee/core/except.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

namespace cee {

	Log::Log(const std::string &name, const std::string &logFile, spdlog::level::level_enum level)
	 : m_LogLevel(level) {
		std::filesystem::path path;
		if (!m_LogFile.empty()) {
			path = std::filesystem::absolute(m_LogFile);
			std::fstream file(path, std::ios::out);
			if (!file)
				throw core::FileError(fmt::format("Cannot open file {}", path.string()));
		} else {
			const char *home = std::getenv("HOME");
			if (home) {
				path = std::filesystem::absolute(std::filesystem::path(home) / (".local/share/cee/" + name + ".log"));
				std::fstream file(path, std::ios::out);
				if (!file)
					throw core::FileError(fmt::format("Cannot open file {}", path.string()));
			} else { 
				path = std::filesystem::absolute("/tmp/" + name + ".log");
				std::fstream file(path, std::ios::out);
				if (!file)
					throw core::FileError(fmt::format("Cannot open file {}", path.string()));
				}
		}
		m_LogFile = path;
		m_Sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		m_Sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(m_LogFile, true));
		m_Sinks[0]->set_pattern("[%T] [%l] %^%n: %v%$");
		m_Sinks[1]->set_pattern("[%T] [%l] %n: %v");

		m_CoreLogger = std::make_shared<spdlog::logger>(name, begin(m_Sinks), end(m_Sinks));
		spdlog::register_logger(m_CoreLogger);
		m_CoreLogger->set_level(m_LogLevel);
		
		m_CoreLogger->info("Writing to log file: {}", m_LogFile);
	}

	Logger Log::CreateChild(const std::string &name) {
		m_CoreLogger->trace("Creating new logger {}", name);

		Logger logger = std::make_shared<spdlog::logger>(name, begin(m_Sinks), end(m_Sinks));
		spdlog::register_logger(logger);
		logger->set_level(m_LogLevel);
		m_Children.push_back(logger);

		return logger;
	}
}

