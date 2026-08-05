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

#include <cee/font/fonts.h>

#include <gtest/gtest.h>

TEST(fontManagment, initManager)
{
	auto fontManager = std::make_unique<cee::font::FontManager>();
}

TEST(fontManagment, createFont)
{
	auto fontManager = std::make_unique<cee::font::FontManager>();
	fontManager->SetDPI(72);
	fontManager->CreateFont("/usr/share/fonts/TTF/DejaVuSans.ttf");
}

TEST(fontManagment, createInstance)
{
	auto fontManager = std::make_unique<cee::font::FontManager>();
	fontManager->SetDPI(72);
	auto font = fontManager->CreateFont("/usr/share/fonts/TTF/DejaVuSans.ttf");
	font->PrepareForSize(12);
}

TEST(fontManagment, loadAscii) {
	auto fontManager = std::make_unique<cee::font::FontManager>();
	fontManager->SetDPI(72);
	auto font = fontManager->CreateFont("/usr/share/fonts/TTF/DejaVuSans.ttf");
	for (int i = 32; i < 127; i++) {
		auto g = font->LoadGlyph(i, 12);
		EXPECT_TRUE(i == g.codepoint);
	}
}

