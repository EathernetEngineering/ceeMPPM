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

#include <algorithm>
#include <cee/mppm/log.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <filesystem>
#include <iterator>
#include <memory>
#include <vector>

namespace cee {
std::shared_ptr<spdlog::logger> Log::s_Logger;
std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::vector<std::weak_ptr<spdlog::logger>> Log::s_Children;

void Log::Init()
{
	std::vector<spdlog::sink_ptr> logSinks;
	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	std::filesystem::path logFile;
	const char *home = std::getenv("HOME");
	if (home) {
		logFile = std::filesystem::absolute(std::filesystem::path(home) / ".local/share/ceeHealth/mppm.log");
	} else { 
		logFile = std::filesystem::absolute("/tmp/mppm.log");
	}
	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true));

	logSinks[0]->set_pattern("[%T] [%l] %^%n: %v%$");
	logSinks[1]->set_pattern("[%T] [%l] %n: %v");

	s_Logger = std::make_shared<spdlog::logger>("MPPM Client", begin(logSinks), end(logSinks));
	spdlog::register_logger(s_Logger);
	s_Logger->set_level(spdlog::level::trace);
	s_Logger->flush_on(spdlog::level::trace);

	s_CoreLogger = std::make_shared<spdlog::logger>("MPPM Core", begin(logSinks), end(logSinks));
	spdlog::register_logger(s_CoreLogger);
	s_CoreLogger->set_level(spdlog::level::trace);
	s_CoreLogger->flush_on(spdlog::level::trace);

	// TODO: Use a ringbuffer sink instead, this wasn't being used anyway
	// spdlog::enable_backtrace(128);
}

void Log::Shutdown()
{
	RemoveDeadChildren();
	for (auto child : s_Children) {
		RemoveLogger(child.lock());
	}
	spdlog::drop(s_Logger->name());
	spdlog::drop(s_CoreLogger->name());
	s_Logger.reset();
	s_CoreLogger.reset();
}

void Log::AddLogger(std::shared_ptr<spdlog::logger> child)
{
	if (child.get() == nullptr) throw std::runtime_error("nullptr passeed to AddLogger");
	child->sinks().insert(std::end(child->sinks()),
						std::begin(s_Logger->sinks()),
						std::end(s_Logger->sinks()));
	s_Children.emplace_back(child);
}

void Log::RemoveLogger(std::shared_ptr<spdlog::logger> childToRemove)
{
	if (childToRemove.get() == nullptr) throw std::runtime_error("nullptr passed to RemoveLogger");

	// Check that the logger to be removed is actually known about by this class
	auto childIt = std::find_if(s_Children.begin(), s_Children.end(),
		[&childToRemove](const std::weak_ptr<spdlog::logger> &child){ 
			if (!child.owner_before(childToRemove) && !childToRemove.owner_before(child))
				return true;
			return false;
		});
	if (childIt == s_Children.end()) {
		CEE_CORE_DEBUG("Attempt to remove logger not registered with mppm");
		/* 
		 * Don't fail out or even warn here becuase sinks aren't private here, and
		 * the behaviour may be wanted even if the logger wasn't added with the
		 * helper function
		 */ 
	}

	// Erase the sinks managed by this from the child
	auto& childSinks = childToRemove->sinks();
	auto& sinks = s_Logger->sinks();
	int n = std::erase_if(childSinks,
		[&sinks](const spdlog::sink_ptr& childSink){
			for (auto it = sinks.begin(); it != sinks.end(); it++) {
				if (!it->owner_before(childSink) && !childSink.owner_before(*it))
					return true;
			}
			return false;
		});
	if (n != sinks.size()) {
		CEE_CORE_WARN("Attempting to remove {}'s sinks from {} removed more than expected",
		   s_Logger->name(),
		   childToRemove->name());
		CEE_CORE_WARN("Removed {} sinks, expected to remove {}",
		   childToRemove->sinks().size(),
		   s_Logger->sinks().size());
	}

	// We got this earlier, but it may still be end()
	if (childIt != s_Children.end()) {
		// This is only a concern if this class ever knew about the child
		s_Children.erase(childIt);
	}
}

void Log::RemoveDeadChildren()
{
	auto newEnd = std::erase_if(s_Children,
		[](const std::weak_ptr<spdlog::logger>& child){
			return child.expired();
		});
}
}

