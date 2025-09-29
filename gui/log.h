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

#ifndef CEE_GUI_LOG_PRIVATE_H_
#define CEE_GUI_LOG_PRIVATE_H_

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace cee {
namespace gui {
	class Log {
	public:
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
}
}

#define CEE_DEBUG(...)       ::cee::gui::Log::GetLogger()->debug(__VA_ARGS__)
#define CEE_TRACE(...)       ::cee::gui::Log::GetLogger()->trace(__VA_ARGS__)
#define CEE_INFO(...)        ::cee::gui::Log::GetLogger()->info(__VA_ARGS__)
#define CEE_WARN(...)        ::cee::gui::Log::GetLogger()->warn(__VA_ARGS__)
#define CEE_ERROR(...)       ::cee::gui::Log::GetLogger()->error(__VA_ARGS__)
#define CEE_CRITICAL(...)    ::cee::gui::Log::GetLogger()->critical(__VA_ARGS__)

#endif

