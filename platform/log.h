/*
 * CeeMPPM
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

#ifndef CEE_PLATFORM_LOG_H_
#define CEE_PLATFORM_LOG_H_

#include <cee/core/log.h>

namespace cee {
namespace platform {
	template<typename T>
	void trace(Logger &l, const T &msg) {
		if (l)
			l->trace(msg);
	}

	template<typename ...Args>
	void trace(Logger &l, spdlog::format_string_t<Args...> fmt, Args &&...args) {
		if (l)
			l->trace(fmt, std::forward<Args>(args)...);
	}

	template<typename T>
	void debug(Logger &l, const T &msg) {
		if (l)
			l->debug(msg);
	}

	template<typename ...Args>
	void debug(Logger &l, spdlog::format_string_t<Args...> fmt, Args &&...args) {
		if (l)
			l->debug(fmt, std::forward<Args>(args)...);
	}

	template<typename T>
	void info(Logger &l, const T &msg) {
		if (l)
			l->info(msg);
	}

	template<typename ...Args>
	void info(Logger &l, spdlog::format_string_t<Args...> fmt, Args &&...args) {
		if (l)
			l->info(fmt, std::forward<Args>(args)...);
	}

	template<typename T>
	void warn(Logger &l, const T &msg) {
		if (l)
			l->warn(msg);
	}

	template<typename ...Args>
	void warn(Logger &l, spdlog::format_string_t<Args...> fmt, Args &&...args) {
		if (l)
			l->warn(fmt, std::forward<Args>(args)...);
	}

	template<typename T>
	void error(Logger &l, const T &msg) {
		if (l)
			l->error(msg);
	}

	template<typename ...Args>
	void error(Logger &l, spdlog::format_string_t<Args...> fmt, Args &&...args) {
		if (l)
			l->error(fmt, std::forward<Args>(args)...);
	}

	template<typename T>
	void critical(Logger &l, const T &msg) {
		if (l)
			l->critical(msg);
	}

	template<typename ...Args>
	void critical(Logger &l, spdlog::format_string_t<Args...> fmt, Args &&...args) {
		if (l)
			l->critical(fmt, std::forward<Args>(args)...);
	}
}
}

#endif

