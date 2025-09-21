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

#include <cee/mppm/log.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <iterator>
#include <filesystem>

namespace cee {
std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::Init()
{
	std::vector<spdlog::sink_ptr> logSinks;
	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	std::filesystem::path logFile;
	const char *home = std::getenv("HOME");
	if (home) {
		logFile = std::filesystem::absolute(std::filesystem::path(home) / ".local/share/ceeHealth/CardiacMonitor.log");
	} else { 
		logFile = std::filesystem::absolute("/tmp/CardiacMonitor.log");
	}
	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true));

	logSinks[0]->set_pattern("[%T] [%l] %^%n: %v%$");
	logSinks[1]->set_pattern("[%T] [%l] %n: %v");

	s_Logger = std::make_shared<spdlog::logger>("MPPM", begin(logSinks), end(logSinks));
	spdlog::register_logger(s_Logger);
	s_Logger->set_level(spdlog::level::trace);
	s_Logger->flush_on(spdlog::level::trace);
}

void Log::AddLogger(std::shared_ptr<spdlog::logger> logger)
{
	logger->sinks().insert(std::end(logger->sinks()), std::begin(s_Logger->sinks()), std::end(s_Logger->sinks()));
}
}

