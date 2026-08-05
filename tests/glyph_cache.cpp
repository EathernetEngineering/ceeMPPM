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

#include <gtest/gtest.h>

TEST(fontGlyphCache, store)
{
	using namespace cee::font;

	GlyphKey k = {
		.sizePt = 12
	};
	Glyph g{ 1, 2, 3 };
	GlyphCache::StoreGlyph(g, k);
	auto ret = GlyphCache::FindGlyph(k);
	EXPECT_TRUE(*ret == g);
	k = {
		.sizePt = 16
	};
	g = { 4, 5, 6 };
	GlyphCache::StoreGlyph(g, k);
	ret = GlyphCache::FindGlyph(k);
	EXPECT_TRUE(*ret == g);
	k = {
		.sizePt = 20
	};
	g = { 7, 8, 9 };
	GlyphCache::StoreGlyph(g, k);
	ret = GlyphCache::FindGlyph(k);
	EXPECT_TRUE(*ret == g);
}

TEST(fontGlyphCache, atlasInvalidatesGlyphs)
{
	using namespace cee::font;

	int data;
	GlyphBitmap bmp = {
		.width = 1,
		.height = 1,
		.pitch = 1,
	};
	bmp.data = reinterpret_cast<uint8_t*>(&data);
	size_t lastPage = -1ul;
	AtlasCache::StoreResult e = AtlasCache::StoreGlyph(bmp);
	GlyphKey k{};
	Glyph g{};
	g.atlasId = e.pageId;
	GlyphCache::StoreGlyph(g, k);
	AtlasCache::ErasePage(e.pageId);
	EXPECT_TRUE(GlyphCache::FindGlyph(k) == nullptr);
}

