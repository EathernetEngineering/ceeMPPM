/*
 * ceeCore
 * Copyright (C) 2026 Chloe Eather
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

#ifndef CEE_CORE_EXCEPT_H_
#define CEE_CORE_EXCEPT_H_

#include <stdexcept>

namespace cee {
namespace core {
	class Error : public std::runtime_error {
	public:
		explicit Error(const std::string &what) : std::runtime_error(what) {}
		explicit Error(const char *what) : std::runtime_error(what) {}
		Error(const Error &other) = default;
		Error &operator=(const Error &other) = default;
	};

	class UsageError : public std::logic_error {
	public:
		explicit UsageError(const std::string &what) : std::logic_error(what) {}
		explicit UsageError(const char *what) : std::logic_error(what) {}
		UsageError(const UsageError &other) = default;
		UsageError &operator=(const UsageError &other) = default;
	};

	class InvalidParameter : public UsageError {
	public:
		explicit InvalidParameter(const std::string &what) : UsageError(what) {}
		explicit InvalidParameter(const char *what) : UsageError(what) {}
		InvalidParameter(const InvalidParameter &other) = default;
		InvalidParameter &operator=(const InvalidParameter &other) = default;
	};

	class InternalError : public Error {
	public:
		explicit InternalError(const std::string &what) : Error(what) {}
		explicit InternalError(const char *what) : Error(what) {}
		InternalError(const InternalError &other) = default;
		InternalError &operator=(const InternalError &other) = default;
	};

	class FileError : public Error {
	public:
		explicit FileError(const std::string &what) : Error(what) {}
		explicit FileError(const char *what) : Error(what) {}
		FileError(const FileError &other) = default;
		FileError &operator=(const FileError &other) = default;
	};
}
}

#endif


