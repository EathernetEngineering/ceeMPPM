/*
 * CeeHealth
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

#ifndef CEE_FONT_EXCEPT_H_
#define CEE_FONT_EXCEPT_H_

#include <format>
#include <stdexcept>

namespace cee {
namespace font {
	class Error : public std::runtime_error {
	public:
		explicit Error(const std::string &what) : std::runtime_error(what) {}
		explicit Error(const char *what) : std::runtime_error(what) {}
		Error(const Error &other) = default;
		Error &operator=(const Error &other) = default;
	};

	class InternalError : public Error {
	public:
		explicit InternalError(const std::string &what) : Error(what) {}
		explicit InternalError(const char *what) : Error(what) {}
		InternalError(const InternalError &other) = default;
		InternalError &operator=(const InternalError &other) = default;
	};

	class GlyphLoadingError : public Error {
	public:
		explicit GlyphLoadingError(const std::string &what) : Error(what) {}
		explicit GlyphLoadingError(const char *what) : Error(what) {}
		GlyphLoadingError(const GlyphLoadingError &other) = default;
		GlyphLoadingError &operator=(const GlyphLoadingError &other) = default;
	};

	class GlyphNotFound : public Error {
	public:
		explicit GlyphNotFound(const std::string &what) : Error(what) {}
		explicit GlyphNotFound(const char *what) : Error(what) {}
		GlyphNotFound(const GlyphNotFound &other) = default;
		GlyphNotFound &operator=(const GlyphNotFound &other) = default;
	};

	class InstanceError : public std::logic_error {
	public:
		explicit InstanceError(const std::string &what) : std::logic_error(what) {}
		explicit InstanceError(const char *what) : std::logic_error(what) {}
		InstanceError(const InstanceError &other) = default;
		InstanceError &operator=(const InstanceError &other) = default;
	};

	class InvalidFontFile : public std::logic_error {
	public:
		explicit InvalidFontFile(const std::string &what) : std::logic_error(what) {}
		explicit InvalidFontFile(const char *what) : std::logic_error(what) {}
		InvalidFontFile(const InvalidFontFile &other) = default;
		InvalidFontFile &operator=(const InvalidFontFile &other) = default;
	};

	class InvalidParameter : public std::logic_error {
	public:
		explicit InvalidParameter(const std::string &what) : std::logic_error(what) {}
		explicit InvalidParameter(const char *what) : std::logic_error(what) {}
		InvalidParameter(const InvalidParameter &other) = default;
		InvalidParameter &operator=(const InvalidParameter &other) = default;
	};

	class PageMissingError : public std::logic_error {
	public:
		explicit PageMissingError(const std::string &what) : std::logic_error(what) {}
		explicit PageMissingError(const char *what) : std::logic_error(what) {}
		explicit PageMissingError(size_t id)
		 : std::logic_error(std::format("Atlas page {} not foud", id)) {}
		PageMissingError(const PageMissingError &other) = default;
		PageMissingError &operator=(const PageMissingError &other) = default;
	};

}
}

#endif

