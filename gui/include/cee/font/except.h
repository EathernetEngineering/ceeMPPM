/*
 * ceeFont
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

#include <cee/core/except.h>

namespace cee {
namespace font {
	class GlyphLoadingError : public core::Error {
	public:
		explicit GlyphLoadingError(const std::string &what) : Error(what) {}
		explicit GlyphLoadingError(const char *what) : Error(what) {}
		GlyphLoadingError(const GlyphLoadingError &other) = default;
		GlyphLoadingError &operator=(const GlyphLoadingError &other) = default;
	};

	class GlyphNotFound : public core::Error {
	public:
		explicit GlyphNotFound(const std::string &what) : Error(what) {}
		explicit GlyphNotFound(const char *what) : Error(what) {}
		GlyphNotFound(const GlyphNotFound &other) = default;
		GlyphNotFound &operator=(const GlyphNotFound &other) = default;
	};

	class InvalidFontFile : public core::Error {
	public:
		explicit InvalidFontFile(const std::string &what) : Error(what) {}
		explicit InvalidFontFile(const char *what) : Error(what) {}
		InvalidFontFile(const InvalidFontFile &other) = default;
		InvalidFontFile &operator=(const InvalidFontFile &other) = default;
	};

	class InstanceError : public core::UsageError {
	public:
		explicit InstanceError(const std::string &what) : UsageError(what) {}
		explicit InstanceError(const char *what) : UsageError(what) {}
		InstanceError(const InstanceError &other) = default;
		InstanceError &operator=(const InstanceError &other) = default;
	};

	class PageMissingError : public core::InvalidParameter {
	public:
		explicit PageMissingError(int pageId, const std::string &what)
		 : InvalidParameter(what), m_PageId(pageId) {
		}
		explicit PageMissingError(int pageId, const char *what)
		 : InvalidParameter(what), m_PageId(pageId) {
		}
		PageMissingError(const PageMissingError &other) = default;
		PageMissingError &operator=(const PageMissingError &other) = default;

		int GetPageId() const noexcept { return m_PageId; }

	private:
		int m_PageId;
	};

}
}

#endif

