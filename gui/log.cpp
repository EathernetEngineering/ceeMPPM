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

#include <log.h>
#include <cee/gui/logger.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <string>

namespace cee {
namespace gui {
	std::shared_ptr<spdlog::logger> Log::s_Logger;

	void Log::Init()
	{
		s_Logger = std::make_shared<spdlog::logger>("GUI");
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		spdlog::drop(s_Logger->name());
		s_Logger.reset();
	}

	std::shared_ptr<spdlog::logger> GetLogger() { return Log::GetLogger(); }
}
}


