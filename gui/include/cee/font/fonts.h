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

#ifndef CEE_FONTS_H_
#define CEE_FONTS_H_

#include <cee/font/types.h>
#include <cee/font/hash.h>

#include <cstdint>

#include <memory>
#include <string>

namespace cee {
namespace font {
	class FontManager {
	private:
		class Impl;

	public:
		FontManager();
		explicit FontManager(const FontManager&) = delete;
		explicit FontManager(const FontManager&&) = delete;
		~FontManager();

		void SetDPI(uint32_t dpi);
		uint32_t GetDPI() const { return m_DPI; }

		std::shared_ptr<class Font> CreateFont(const std::string &file);

		const AtlasPage& GetAtlasPage(int n);

	public:
		static FontManager *Get() { return s_Instance; }

	private:
		uint32_t m_DPI;

	private:
		static FontManager *s_Instance;
		std::unique_ptr<Impl> impl;
	};

	class Font {
	private:
		class Impl;
		explicit Font(const std::string &file, FontID id, std::unique_ptr<Impl> &&impl);

	public:
		void EnsureGlyph(uint32_t codepoint, int sizepPt);
		const Glyph& GetGlyph(uint32_t codepoint, int sizePt) const;
		const Glyph& LoadGlyph(uint32_t codepoint, int sizePt);
		const Glyph& TryGetGlyph(uint32_t codepoint, int sizePt) const;
		const Glyph& TryLoadGlyph(uint32_t codepoint, int sizePt);

		void PrepareForSize(int sizePt);

		float GetAscent(int sizePt) const;
		float GetDescent(int sizePt) const;
		float GetLineHeight(int sizePt) const;
		float GetLineGap(int sizePt) const;

		FontID GetID() const { return m_Id; }

	public:
		~Font();

	private:
		std::string m_File;
		FontID m_Id;

	private:
		std::unique_ptr<Impl> impl;
		friend class FontManager;
	};
}
}

#endif

