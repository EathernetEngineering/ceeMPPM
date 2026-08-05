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

TEST(fontAtlasCache, store)
{
	using namespace cee::font;

	int data;
	GlyphBitmap bmp = {
		.width = 1,
		.height = 1,
		.pitch = 1,
	};
	bmp.data = reinterpret_cast<uint8_t*>(&data);
	std::vector<AtlasPageID> pages;
	size_t lastPage = -1ul;
	for (long i = 0; i < 4096; i++) {
		auto e = AtlasCache::StoreGlyph(bmp);
		if (lastPage != e.pageId) {
			pages.push_back(e.pageId);
			lastPage = e.pageId;
		}
		EXPECT_EQ(e.pageId, lastPage);
		EXPECT_EQ(e.width, bmp.width);
		EXPECT_EQ(e.height, bmp.height);
		auto &page = AtlasCache::GetPage(e.pageId);
		EXPECT_EQ(page.id, e.pageId);
		ASSERT_EQ(page.width, ATLAS_WIDTH);
		ASSERT_EQ(page.height, ATLAS_HEIGHT);
	}
	for (auto id : pages) {
		AtlasCache::ErasePage(id);
	}
	EXPECT_TRUE(AtlasCache::GetPageCount() == 0);
}

