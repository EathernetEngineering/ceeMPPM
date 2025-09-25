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

#include <gtest/gtest.h>
#include <spdlog/sinks/ringbuffer_sink.h>

#include <algorithm>
#include <memory>
#include <vector>

template<typename T, typename Alloc>
inline testing::AssertionResult VectorContains(const std::vector<T, Alloc> &vec,
									  const std::vector<T, Alloc> &range)
{
	for (const auto& range_n : range) {
		if (std::find(std::begin(vec), std::end(vec), range_n) == std::end(vec))
			return testing::AssertionFailure();
	}
	return testing::AssertionSuccess();
}

template<typename T, typename Alloc>
inline testing::AssertionResult VectorDoesNotContain(const std::vector<T, Alloc> &vec,
									  const std::vector<T, Alloc> &range)
{
	for (const auto& range_n : range) {
		if (std::find(std::begin(vec), std::end(vec), range_n) != std::end(vec))
			return testing::AssertionFailure();
	}
	return testing::AssertionSuccess();
}

TEST(mppmLog, initShutdownLogger)
{
	cee::Log::Init();
	ASSERT_NE(cee::Log::GetLogger().get(), nullptr);
	/*
	 * FIXME: Remove magic numer.
	 *
	 * The value here is 2 becuase we expect to have a sink printing to stdout
	 * and a sink printing to a log file, This may change in the future so maybe
	 * have a define or something so that this test won't fail if and when the
	 * amount of sinks changes or becomes dynamic.
	 */
	EXPECT_EQ(cee::Log::GetLogger()->sinks().size(), 2);

	cee::Log::Shutdown();
	EXPECT_EQ(cee::Log::GetLogger().get(), nullptr);
}

TEST(mppmLog, addEmptyLogger)
{
	cee::Log::Init();

	auto mppmLogger = cee::Log::GetLogger();
	std::shared_ptr<spdlog::logger> emptyLogger = std::make_shared<spdlog::logger>("emptyLogger");

	cee::Log::AddLogger(emptyLogger);
	EXPECT_TRUE(VectorContains(emptyLogger->sinks(), mppmLogger->sinks())) <<
		"Previous call to cee::Log::AddLogger(emptyLogger) should add its own sinks to " <<
		std::endl << "logger1, which previously had no sinks, both should be the same";

	cee::Log::Shutdown();
}

TEST(mppmLog, addLoggerWithOwnSinks)
{
	cee::Log::Init();

	auto mppmLogger = cee::Log::GetLogger();
	std::shared_ptr<spdlog::logger> loggerWithOwnSinks;
	loggerWithOwnSinks = std::make_shared<spdlog::logger>("loggerWithOwnSinks");
	loggerWithOwnSinks->sinks().emplace_back(std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(4));
	loggerWithOwnSinks->sinks().emplace_back(std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(5));

	std::vector<spdlog::sink_ptr> testLoggerSinksCopy(loggerWithOwnSinks->sinks());

	cee::Log::AddLogger(loggerWithOwnSinks);

	EXPECT_TRUE(VectorContains(loggerWithOwnSinks->sinks(), mppmLogger->sinks()));
	EXPECT_TRUE(VectorContains(loggerWithOwnSinks->sinks(), testLoggerSinksCopy));
	EXPECT_EQ(loggerWithOwnSinks->sinks().size(), mppmLogger->sinks().size() + testLoggerSinksCopy.size());

	cee::Log::Shutdown();
}

TEST(mppmLog, removeLogger)
{
	cee::Log::Init();

	auto mppmLogger = cee::Log::GetLogger();
	std::shared_ptr<spdlog::logger> emptyTestLogger = std::make_shared<spdlog::logger>("emptyTestLogger");

	cee::Log::AddLogger(emptyTestLogger);
	ASSERT_TRUE(VectorContains(emptyTestLogger->sinks(), mppmLogger->sinks()));

	cee::Log::RemoveLogger(emptyTestLogger);
	EXPECT_EQ(emptyTestLogger->sinks().size(), 0);

	cee::Log::Shutdown();
}

TEST(mppmLog, removeLoggerPreservesSinks)
{
	cee::Log::Init();

	auto mppmLogger = cee::Log::GetLogger();
	std::shared_ptr<spdlog::logger> testLoggerWithSinks;
	std::vector<spdlog::sink_ptr> testLoggerSinksCopy;
	spdlog::sink_ptr sink0 = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(3);
	spdlog::sink_ptr sink1 = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(4);
	spdlog::sink_ptr sink2 = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(5);
	spdlog::sink_ptr sink3 = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(6);
	testLoggerSinksCopy.push_back(sink0);
	testLoggerSinksCopy.push_back(sink1);
	testLoggerWithSinks = std::make_shared<spdlog::logger>("testLoggerWithSinks",
														testLoggerSinksCopy.begin(),
														testLoggerSinksCopy.end());

	cee::Log::AddLogger(testLoggerWithSinks);

	testLoggerWithSinks->sinks().push_back(sink2);
	testLoggerSinksCopy.push_back(sink2);
	testLoggerWithSinks->sinks().push_back(sink3);
	testLoggerSinksCopy.push_back(sink3);

	cee::Log::RemoveLogger(testLoggerWithSinks);

	// Make sure the funtion actually removes the sinks from the child logger
	EXPECT_TRUE(VectorDoesNotContain(testLoggerWithSinks->sinks(), mppmLogger->sinks()));
	// Make sure the funtion doesn't touch any sinks owned by the child
	EXPECT_TRUE(VectorContains(testLoggerWithSinks->sinks(), testLoggerSinksCopy));

	cee::Log::Shutdown();
}

TEST(mppmLog, shutdownRemovesSinksFromChildren)
{
	cee::Log::Init();

	auto mppmLogger = cee::Log::GetLogger();
	std::shared_ptr<spdlog::logger> testLogger = std::make_shared<spdlog::logger>("testLogger");

	cee::Log::AddLogger(testLogger);
	EXPECT_NE(testLogger->sinks().size(), 0);

	cee::Log::Shutdown();
	
	EXPECT_EQ(testLogger->sinks().size(), 0);
}

