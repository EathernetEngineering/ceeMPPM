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

#include <cee/font/fonts.h>
#include <cee/font/except.h>
#include <fontCache.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

#include <filesystem>
#include <format>
#include <mutex>
#include <utility>

namespace cee {
namespace font {
	class FontInstance {
	public:
		explicit FontInstance(FT_Face face, int sizePt);
		FontInstance(FontInstance &&other);
		~FontInstance();

		const Glyph& LoadGlyph(FT_Face face, const GlyphKey &k, uint32_t codepoint);
		void DeleteGlyph(const GlyphKey &k, uint32_t codepoint);

		float GetAscent() const { return m_Size->metrics.ascender >> 6; };
		float GetDescent() const { return m_Size->metrics.descender >> 6; };
		float GetLineHeight() const { return m_Size->metrics.height >> 6; };
		float GetLineGap() const {
			float ascent = m_Size->metrics.ascender >> 6;
			float descent = m_Size->metrics.ascender >> 6;
			float lineHeight = m_Size->metrics.ascender >> 6;
			return lineHeight - (ascent - descent);
		};

		int GetSizePt() const { return m_SizePt; }

	private:
		GlyphKey CreateGlyphKey(FT_Face face, FontID id, uint32_t codepoint) {
			int idx = FT_Get_Char_Index(face, codepoint);
			if (idx == 0 && codepoint != 0xFFFD) {
				throw GlyphNotFound(std::format("CreateGlyphKey(): Codepoint U+{:X} not found in font {}", codepoint, id));
			}
			return GlyphKey {
				.fontId = id,
				.faceGlyphIndex = idx,
				.sizePt = m_SizePt,
				.flags = 0
			};
		}

	private:
		int m_SizePt;
		FT_Size m_Size;
	};

	FontManager *FontManager::s_Instance = nullptr;

	class FontManager::Impl {
	public:
		Impl();
		~Impl();

		std::shared_ptr<Font> CreateFont(const char *file);

	public:
		mutable std::mutex _Mutex;
		FT_Library _Lib;
		size_t _FontIdCounter;
	};

	class Font::Impl {
	public:
		explicit Impl(FT_Library lib, const char *file);
		~Impl();

		float GetAscent(int sizePt) const;
		float GetDescent(int sizePt) const;
		float GetLineHeight(int sizePt) const;
		float GetLineGap(int sizePt) const;

		void EnsureGlyph(FontID id, uint32_t codepoint, int sizepPt);
		const Glyph& GetGlyph(FontID id, uint32_t codepoint, int sizePt) const;

		void PrepareForSize(int size);

	private:
		FontInstance *GetInstanceForSize(int sizePt) {
			return const_cast<FontInstance *>(std::as_const(*this).GetInstanceForSize(sizePt));
		}

		const FontInstance *GetInstanceForSize(int sizePt) const {
			if (sizePt == _LastUsedSize && _LastUsedSize > 0)
				return &_Instances[_LastUsedInstance];
			for (size_t i = 0; i < _Instances.size(); ++i) {
				if (_Instances[i].GetSizePt() == sizePt) {
					_LastUsedSize = sizePt;
					_LastUsedInstance = i;
					return &_Instances[i];
				}
			}
			return nullptr;
		}

	public:
		mutable std::mutex _Mutex;
		FT_Face _Face;
		std::vector<FontInstance> _Instances;

		mutable int _LastUsedSize = -1;
		mutable int _LastUsedInstance;
	};

	FontManager::Impl::Impl()
	 : _FontIdCounter(0)
	{
		int error = FT_Init_FreeType(&_Lib);
		if (error)
			throw InternalError(std::format("FontManager(): Failed to initialize freetype ({})", error));
	};

	FontManager::Impl::~Impl() {
		FT_Done_FreeType(_Lib);
	}

	std::shared_ptr<Font> FontManager::Impl::CreateFont(const char *file) {
		std::lock_guard lock(_Mutex);
		auto fontImpl = std::unique_ptr<Font::Impl>(new Font::Impl(_Lib, file));
		return std::shared_ptr<Font>(new Font(file, _FontIdCounter++, std::move(fontImpl)));
	}

	Font::Impl::Impl(FT_Library lib, const char *file) {
		if (!std::filesystem::exists(file))
			throw InvalidFontFile(std::format("CreateFont(): File \"{}\" does not exist", file));

		int error = FT_New_Face(lib, file, 0, &_Face);
		if (error)
			throw InternalError(std::format("CreateFont(): Failed to initialize font {} ({})", file, error));
	}

	Font::Impl::~Impl() {
		_Instances.clear();
		FT_Done_Face(_Face);
	}

	float Font::Impl::GetAscent(int sizePt) const {
		std::lock_guard lock(_Mutex);
		const FontInstance *inst = GetInstanceForSize(sizePt);
		if (!inst)
			throw InvalidParameter("GetAscent(): Cannot get metrics for font with size that has not been loaded");
		return inst->GetAscent();
	}

	float Font::Impl::GetDescent(int sizePt) const {
		std::lock_guard lock(_Mutex);
		const FontInstance *inst = GetInstanceForSize(sizePt);
		if (!inst)
			throw InvalidParameter("GetDescent(): Cannot get metrics for font with size that has not been loaded");
		return inst->GetDescent();
	}

	float Font::Impl::GetLineHeight(int sizePt) const {
		std::lock_guard lock(_Mutex);
		const FontInstance *inst = GetInstanceForSize(sizePt);
		if (!inst)
			throw InvalidParameter("GetLineHeight(): Cannot get metrics for font with size that has not been loaded");
		return inst->GetLineHeight();
	}

	float Font::Impl::GetLineGap(int sizePt) const {
		std::lock_guard lock(_Mutex);
		const FontInstance *inst = GetInstanceForSize(sizePt);
		if (!inst)
			throw InvalidParameter("GetLineGap(): Cannot get metrics for font with size that has not been loaded");
		return inst->GetLineGap();
	}

	void Font::Impl::EnsureGlyph(FontID id, uint32_t codepoint, int sizePt) {
		int idx = FT_Get_Char_Index(_Face, codepoint);
		GlyphKey key = {
			.fontId = id,
			.faceGlyphIndex = idx,
			.sizePt = sizePt,
			.flags = 0
		};
		const Glyph *ptr = GlyphCache::FindGlyph(key);
		if (ptr != nullptr)
			return;

		FontInstance *inst = GetInstanceForSize(sizePt);
		if (inst == nullptr) {
			PrepareForSize(sizePt);
			inst = GetInstanceForSize(sizePt);
			if (inst == nullptr)
				throw InternalError("EnsureGlyph(): Failed to create font instance");
		}
		GlyphKey k = {
			.fontId = id,
			.faceGlyphIndex = idx,
			.sizePt = sizePt,
			.flags = 0
		};
		inst->LoadGlyph(_Face, k, codepoint);
	}

	const Glyph& Font::Impl::GetGlyph(FontID id, uint32_t codepoint, int sizePt) const {
		int idx = FT_Get_Char_Index(_Face, codepoint);
		GlyphKey key = {
			.fontId = id,
			.faceGlyphIndex = idx,
			.sizePt = sizePt,
			.flags = 0
		};
		const Glyph *ptr = GlyphCache::FindGlyph(key);
		if (ptr == nullptr)
			throw InvalidParameter(std::format("GetGlyph(): Codepoint U+{:X} not loaded in font {}", codepoint, id));
		return *ptr;
	}

	void Font::Impl::PrepareForSize(int sizePt) {
		if (GetInstanceForSize(sizePt) != nullptr) {
			throw InvalidParameter(std::format("Instance for font size {} already exists", sizePt));
		}
		_Instances.emplace_back(_Face, sizePt);
	}

	FontInstance::FontInstance(FT_Face face, int sizePt)
	 : m_SizePt(sizePt)
	{
		int dpi = FontManager::Get()->GetDPI();
		int error = FT_New_Size(face, &m_Size);
		if (error != 0)
			throw InternalError("FontInstance(): Failed to create size object");
		error = FT_Activate_Size(m_Size);
		if (error != 0)
			throw InternalError("FontInstance(): Failed to switch active font size");
		error = FT_Set_Char_Size(face, 0, sizePt * 64, dpi, dpi);
		if (error != 0)
			throw InvalidParameter("FontInstance(): Failed to set font size");
	}

	FontInstance::FontInstance(FontInstance &&other) {
		m_SizePt = other.m_SizePt;
		m_Size = other.m_Size;
		other.m_Size = nullptr;
	}

	FontInstance::~FontInstance() {
		if (m_Size)
			FT_Done_Size(m_Size);
	}

	const Glyph &FontInstance::LoadGlyph(FT_Face face, const GlyphKey &k, uint32_t codepoint) {
		Glyph glyph;

		int error = FT_Activate_Size(m_Size);
		if (error != 0)
			throw InternalError("LoadGlyph(): Failed to switch active font size");

		error = FT_Load_Glyph(face, k.faceGlyphIndex, FT_LOAD_RENDER);
		if (error != 0)
			throw InternalError("LoadGlyph(): Failed to rasterize glyph");

		GlyphBitmap bmp = {
			.width = face->glyph->bitmap.width,
			.height = face->glyph->bitmap.rows,
			.pitch = face->glyph->bitmap.pitch,
			.data = face->glyph->bitmap.buffer
		};
		AtlasCache::StoreResult entry = AtlasCache::StoreGlyph(bmp);
		glyph.codepoint = codepoint;
		glyph.width = face->glyph->metrics.width >> 6;
		glyph.height = face->glyph->metrics.height >> 6;
		glyph.bearingX = face->glyph->metrics.horiBearingX >> 6;
		glyph.bearingY = face->glyph->metrics.horiBearingY >> 6;
		glyph.advance = face->glyph->advance.x >> 6;
		glyph.atlasX = entry.atlasX;
		glyph.atlasY = entry.atlasY;
		glyph.atlasId = entry.pageId;

		return GlyphCache::StoreGlyph(std::move(glyph), k);
	}

	void FontInstance::DeleteGlyph(const GlyphKey &k, uint32_t codepoint) {
		const Glyph *cacheLookup = GlyphCache::FindGlyph(k);
		if (!cacheLookup)
			throw InvalidParameter(std::format("DeleteGlyph(): Codepoint U+{:X} not loaded in font {}",
						codepoint, k.fontId));
		GlyphCache::ReleaseGlyph(k);
	}

	FontManager::FontManager() {
		if (s_Instance != nullptr)
			throw InstanceError("FontManager(): Only one instance of class FontManager can be created");
		s_Instance = this;

		impl = std::make_unique<Impl>();

		m_DPI = 72;
	}

	FontManager::~FontManager() {
		s_Instance = nullptr;
	}

	void FontManager::SetDPI(uint32_t dpi) {
		m_DPI = dpi;
	}

	std::shared_ptr<Font> FontManager::CreateFont(const std::string &file) {
		return impl->CreateFont(file.c_str());
	}

	Font::Font(const std::string &file, FontID id, std::unique_ptr<Impl> &&impl)
	 : m_File(file), m_Id(id), impl(std::move(impl))
	{
	}

	Font::~Font() {
	}

	void Font::EnsureGlyph(uint32_t codepoint, int sizePt) {
		return impl->EnsureGlyph(m_Id, codepoint, sizePt);
	}

	const Glyph &Font::GetGlyph(uint32_t codepoint, int sizePt) const {
		const auto &glyph =  impl->GetGlyph(m_Id, codepoint, sizePt);
		if (glyph.codepoint == 0 || glyph.codepoint == 0xFFFD)
			throw GlyphNotFound(std::format("GetGlyph(): Codepoint U+{:X} not found in font {}", codepoint, m_Id));
		return glyph;
	}

	const Glyph &Font::LoadGlyph(uint32_t codepoint, int sizePt) {
		impl->EnsureGlyph(m_Id, codepoint, sizePt);
		const auto &glyph = impl->GetGlyph(m_Id, codepoint, sizePt);
		if (glyph.codepoint == 0 || glyph.codepoint == 0xFFFD)
			throw GlyphNotFound(std::format("GetGlyph(): Codepoint U+{:X} not found in font {}", codepoint, m_Id));
		return glyph;
	}

	const Glyph &Font::TryGetGlyph(uint32_t codepoint, int sizePt) const {
		return impl->GetGlyph(m_Id, codepoint, sizePt);
	}

	const Glyph &Font::TryLoadGlyph(uint32_t codepoint, int sizePt) {
		impl->EnsureGlyph(m_Id, codepoint, sizePt);
		return impl->GetGlyph(m_Id, codepoint, sizePt);
	}

	void Font::PrepareForSize(int sizePt) {
		impl->PrepareForSize(sizePt);
	}


	const AtlasPage& FontManager::GetAtlasPage(int n) {
		return AtlasCache::GetPage(n);
	}

	float Font::GetAscent(int sizePt) const {
		return impl->GetAscent(sizePt);
	}

	float Font::GetDescent(int sizePt) const {
		return impl->GetDescent(sizePt);
	}

	float Font::GetLineHeight(int sizePt) const {
		return impl->GetLineHeight(sizePt);
	}

	float Font::GetLineGap(int sizePt) const {
		return impl->GetLineGap(sizePt);
	}
}
}

