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
#include <cee/hal/hal.h>
#include <cee/hal/logger.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <string>

#include <cstdarg>
#include <cstring>
#include <cstdlib>

static bool g_Initialized = false;

static std::string Format(const char *fmt, va_list args)
{
	std::size_t len = 0; 
	/* 
	 * A copy of the list must be made in case the buffer needs reallocation.
	 * This must be done before consuming the arguments from the list.
	 */
	va_list argsCopy;
	/* 
	 * Always preempt this amount, a page doesn't cost significantly more than
	 * any less, we can always allocate more if needed, and this is a fair size
	 * for a print call so we won't need to reallocate, saving another
	 * `malloc()` in most cases.
	 */
	std::size_t outStrSize = 4096; 
	char *outStr = reinterpret_cast<char *>(std::calloc(outStrSize, sizeof(char)));
	if (!outStr) {
		return {};
	}

	va_copy(argsCopy, args);

	len = std::vsnprintf(outStr, outStrSize, fmt, args);
	if (len < 0) {
		free(outStr);
		CEE_ERROR("An error occured formattig message, printing unformatted message:");
		return fmt;
	} else if (len > (outStrSize - 1)) {
		outStrSize = (len * -1) + 1;
		/* 
		 * This is a function for logging, any string longer than 1M chars is
		 * probably not intentional, so lets just not even try.
		 */
		if (len > 0x100000) {
			free(outStr);
			CEE_ERROR("Attempting to print a message > 1M characters");
			return {};
		}

		/* 
		 * Only try again once, the correct amount of memory should be allocated,
		 * if not there is an error anyway.
		 */
		free(outStr);
		outStr = reinterpret_cast<char *>(calloc(outStrSize, sizeof(char)));
		if (!outStr) {
			return {};
		}
		len = std::vsnprintf(outStr, outStrSize, fmt, argsCopy);
		if (len < 0) {
			free(outStr);
			/* 
			 * There is no reason to completely hide the message so just print `fmt` so
			 * the user can at least try to inerpret the message, and a lot of strings
			 * don't even need formatting. At this point we have also ruled out the
			 * possibilty of the string being excessively long so this should be safe.
			 */
			CEE_ERROR("An error occured formattig message, printing unformatted message:");
			return fmt;
		}
	}

	std::string result(outStr, len);

	free(outStr);

	return result;
}

extern "C" void HALLogInit(void)
{
	cee::hal::Log::Init();
}

extern "C" void HALLogShutdown(void)
{
	cee::hal::Log::Shutdown();
}

extern "C" void HALLogSetLevel(LogLevel level)
{
	spdlog::level::level_enum spdLevel = spdlog::level::trace;
	switch (level) {
		case CEE_LOG_LEVEL_DEBUG:
			spdLevel = spdlog::level::debug;
			break;

		case CEE_LOG_LEVEL_TRACE:
			spdLevel = spdlog::level::trace;
			break;

		case CEE_LOG_LEVEL_INFO:
			spdLevel = spdlog::level::info;
			break;

		case CEE_LOG_LEVEL_WARN:
			spdLevel = spdlog::level::warn;
			break;

		case CEE_LOG_LEVEL_ERROR:
			spdLevel = spdlog::level::err;
			break;

		case CEE_LOG_LEVEL_CRITICAL:
			spdLevel = spdlog::level::critical;
			break;

		default:
			CEE_WARN("Invalid LogLevel given, falling back to debug level!");
	};

	cee::hal::Log::GetLogger()->set_level(spdLevel);
	cee::hal::Log::GetLogger()->flush_on(spdLevel);
}

extern "C" void HALDebugMessage(const char *fmt, ...)
{
	va_list args;

	if (std::strlen(fmt) == 0) {
		cee::hal::Log::GetLogger()->debug("[Empty String]");
		return;
	}

	if (std::strchr(fmt, '%') == NULL) {
		cee::hal::Log::GetLogger()->debug(fmt);
		return;
	}

	va_start(args, fmt);
	std::string msg = Format(fmt, args);
	va_end(args);

	cee::hal::Log::GetLogger()->debug("{}", msg);
}

extern "C" void HALTraceMessage(const char *fmt, ...)
{
	va_list args;

	if (std::strlen(fmt) == 0) {
		cee::hal::Log::GetLogger()->trace("[Empty String]");
	}

	if (std::strchr(fmt, '%') == NULL) {
		cee::hal::Log::GetLogger()->trace(fmt);
		return;
	}

	va_start(args, fmt);
	std::string msg = Format(fmt, args);
	va_end(args);

	cee::hal::Log::GetLogger()->trace("{}", msg);
}

extern "C" void HALInfoMessage(const char *fmt, ...)
{
	va_list args;

	if (std::strlen(fmt) == 0) {
		cee::hal::Log::GetLogger()->info("[Empty String]");
	}

	if (std::strchr(fmt, '%') == NULL) {
		cee::hal::Log::GetLogger()->info(fmt);
		return;
	}

	va_start(args, fmt);
	std::string msg = Format(fmt, args);
	va_end(args);

	cee::hal::Log::GetLogger()->info("{}", msg);
}

extern "C" void HALWarnMessage(const char *fmt, ...)
{
	va_list args;

	if (std::strlen(fmt) == 0) {
		cee::hal::Log::GetLogger()->warn("[Empty String]");
	}

	if (std::strchr(fmt, '%') == NULL) {
		cee::hal::Log::GetLogger()->warn(fmt);
		return;
	}

	va_start(args, fmt);
	std::string msg = Format(fmt, args);
	va_end(args);

	cee::hal::Log::GetLogger()->warn("{}", msg);
}

extern "C" void HALErrorMessage(const char *fmt, ...)
{
	va_list args;

	if (std::strlen(fmt) == 0) {
		cee::hal::Log::GetLogger()->error("[Empty String]");
	}

	if (std::strchr(fmt, '%') == NULL) {
		cee::hal::Log::GetLogger()->error(fmt);
		return;
	}

	va_start(args, fmt);
	std::string msg = Format(fmt, args);
	va_end(args);

	cee::hal::Log::GetLogger()->error("{}", msg);
}

extern "C" void HALCriticalMessage(const char *fmt, ...)
{
	va_list args;

	if (std::strlen(fmt) == 0) {
		cee::hal::Log::GetLogger()->critical("[Empty String]");
	}

	if (std::strchr(fmt, '%') == NULL) {
		cee::hal::Log::GetLogger()->critical(fmt);
		return;
	}

	va_start(args, fmt);
	std::string msg = Format(fmt, args);
	va_end(args);

	cee::hal::Log::GetLogger()->critical("{}", msg);
}

namespace cee {
namespace hal {
	std::shared_ptr<spdlog::logger> Log::s_Logger;

	void Log::Init()
	{
		s_Logger = std::make_shared<spdlog::logger>("HAL");
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);


		g_Initialized = true;
	}

	void Log::Shutdown()
	{
		spdlog::drop(s_Logger->name());
		s_Logger.reset();
	}

	std::shared_ptr<spdlog::logger> GetLogger() { return Log::GetLogger(); }
}
}

