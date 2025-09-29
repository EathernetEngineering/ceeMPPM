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

#include <cee/mppm/parser.h>

#include <gtest/gtest.h>

static const char *const glslTestStringVersionLine = "#version 450";
static const int glslTestStringVersionNum = 450;
static const char *const glslTestString =
	"#version 450\n"
	"\n"
	"int main() {"
	"\t\n"
	"\n"
	"}";

TEST(mppmParse, ParseGLSL)
{
	cee::GLSLParser parser;
	std::string_view glsl(glslTestString);
	parser.Parse(glsl);

	ASSERT_EQ(parser.VersionString().length(), std::strlen(glslTestStringVersionLine));
	// convert to std::string for null termionation for strcmp();
	std::string readVersionString(parser.VersionString().data(), parser.VersionString().length());
	ASSERT_STREQ(glslTestStringVersionLine, readVersionString.c_str());
	ASSERT_EQ(glslTestStringVersionNum, parser.VersionNum());
}

