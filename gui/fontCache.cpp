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

#include <fontCache.h>
#include <cee/font/except.h>

#include <fmt/format.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace cee {
namespace font {

	std::mutex GlyphCache::s_Mutex;
	std::unordered_map<GlyphKey, Glyph, GlyphCache::GlyphKeyHash> GlyphCache::s_Glyphs;

	std::mutex AtlasCache::s_Mutex;
	std::list<AtlasCache::Entry> AtlasCache::s_Entries;
	AtlasPageID AtlasCache::s_IDCounter = 0;

	const Glyph *GlyphCache::FindGlyph(const GlyphKey &key) {
		std::lock_guard lock(s_Mutex);
		auto it = s_Glyphs.find(key);
		return (it != s_Glyphs.end()) ? &it->second : nullptr;
	}

	const Glyph &GlyphCache::StoreGlyph(const Glyph &glyph, const GlyphKey &key) {
		std::lock_guard lock(s_Mutex);
		if (s_Glyphs.contains(key)) {
			return (s_Glyphs[key] = glyph);
		} else {
			s_Glyphs.emplace(key, glyph);
			return s_Glyphs.at(key);
		}
	}

	const Glyph &GlyphCache::StoreGlyph(Glyph &&glyph, const GlyphKey &key) {
		std::lock_guard lock(s_Mutex);
		if (s_Glyphs.contains(key)) {
			return (s_Glyphs[key] = std::move(glyph));
		} else {
			s_Glyphs.emplace(key, std::move(glyph));
			return s_Glyphs.at(key);
		}
	}

	void GlyphCache::ReleaseGlyph(const GlyphKey &key) {
		std::lock_guard lock(s_Mutex);
		s_Glyphs.erase(key);
		// TODO: Notifty atlas
	}

	void GlyphCache::invalidate_if(std::function<bool(const std::pair<GlyphKey, const Glyph> &g)> predicate) {
		std::lock_guard lock(s_Mutex);
		std::erase_if(s_Glyphs, predicate);
	}

	AtlasCache::StoreResult AtlasCache::StoreGlyph(const GlyphBitmap &bmp) {
		std::lock_guard vecLock(s_Mutex);
		Entry *e = nullptr;
		if (s_Entries.empty())
			e = &AddPage();
		if (!e)
			e = &s_Entries.back();
		AtlasPage *page = &e->first;
		AtlasData *data = &e->second;
		std::unique_lock pageLock(page->mutex);
		if (data->penX + bmp.width + 1 > page->width) {
			data->penY += data->lineHeight;
			data->lineHeight = bmp.height + 1;
			data->penX = 1;
		} else {
			data->lineHeight = std::max(bmp.height + 1, data->lineHeight);
		}
		if (data->lineHeight + data->penY > page->height) {
			pageLock.unlock();
			e = &AddPage();
			page = &e->first;
			data = &e->second;
			pageLock = std::unique_lock(page->mutex);
		}
		if (data->penX + bmp.width + 1 > page->width ||
				data->penY + bmp.height + 1 > page->height) {
			std::string msg = fmt::format("StoreGlyph(): Failed to store glyph in atlas. "
					"Size: {}x{} (max: {}x{})",
					bmp.width, bmp.height, page->width, page->height);
			throw GlyphLoadingError(msg);
		}
		StoreResult result = {
			.pageId = page->id,
			.atlasX = data->penX,
			.width = bmp.width,
			.atlasY = data->penY,
			.height = bmp.height
		};

		auto dst = data->data.begin() + (data->penX + data->penY * page->width);
		for (int row = 0; row < bmp.height; row++) {
			const uint8_t *sourceRow;
			if (bmp.pitch >= 0)
				sourceRow = bmp.data + row * bmp.pitch;
			else
				sourceRow = bmp.data + (bmp.height - 1 - row) * (-bmp.pitch);

			std::copy_n(sourceRow, bmp.width, dst);
			dst += page->width;
		}
		data->penX += bmp.width + 1;
		page->version++;

		return result;
	}

	const AtlasPage &AtlasCache::GetPage(AtlasPageID id) {
		std::lock_guard lock(s_Mutex);
		auto it = std::find_if(s_Entries.begin(), s_Entries.end(), [id](const Entry &entry){
				return entry.first.id == id && entry.second.valid;
			});

		if (it == s_Entries.end()) {
			throw PageMissingError(id, "Missing page");
		}
		return it->first;
	}

	void AtlasCache::ErasePage(AtlasPageID id) {
		std::lock_guard lock(s_Mutex);
		GlyphCache::invalidate_if([id](const std::pair<GlyphKey, Glyph> &g){
				return g.second.atlasId == id;
			});

		auto it = std::find_if(s_Entries.begin(), s_Entries.end(),
				[&](const Entry &entry){ return entry.first.id == id; });
		if (it == s_Entries.end())
			throw PageMissingError(id, "Missing page");
		{
			auto  &[ page, data ] = *it;
			std::unique_lock pageLock(page.mutex);
			data.valid = false;
		}
		s_Entries.erase(it);
	}

	AtlasCache::Entry &AtlasCache::AddPage() {
		AtlasData data = {
			.penX = 1,
			.penY = 1,
			.lineHeight = 1,
			.valid = true,
			.data = std::vector<uint8_t>(ATLAS_WIDTH*ATLAS_HEIGHT, 0)
		};
		AtlasPage page(ATLAS_WIDTH, ATLAS_HEIGHT, 1, s_IDCounter++, data.data.data());
		auto& entry = s_Entries.emplace_back(std::move(page), std::move(data));

		return entry;
	}
}
}

