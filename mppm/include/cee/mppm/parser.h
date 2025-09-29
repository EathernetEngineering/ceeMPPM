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

#ifndef CEE_PARSER_H_
#define CEE_PARSER_H_

#include <cee/mppm/files.h>

#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cee {
	/*
	 **************************************************
	 ************ Parser Specialisations ***********
	 **************************************************
	 */
template<typename charT = char,
	typename Tr = std::char_traits<charT>,
	typename Alloc = std::allocator<charT>>
class GLSLParser {
public:
	using string_type = std::basic_string<charT, Tr, Alloc>;
	using string_view_type = std::basic_string_view<charT, Tr>;
public:
	// Base constructor/destructor
	GLSLParser() = default;
	~GLSLParser() = default;

	// copy constructor/assignment operator (deleted)
	GLSLParser(const GLSLParser& other) = delete;
	GLSLParser& operator=(const GLSLParser& other) = delete;

	// move constructor/assignment operator
	GLSLParser(GLSLParser&& other) = default;
	GLSLParser& operator=(GLSLParser&& other) = default;

	// Swap operation
	void Swap(GLSLParser& other) noexcept {
		if (this == &other)
			return;

		m_VersionString.swap(other.m_VersionString);
		m_GLSLString.swap(other.m_GLSLString);
	}

	// Parse the file and store its entirety to be found in string()
	template<typename C, typename CTr = std::char_traits<C>>
	requires (std::is_convertible_v<C, charT> &&
		std::is_same_v<Tr, CTr>)
	void Parse(files::BasicFile<C, CTr>& file) {
		m_GLSLString.resize(file.GetSize(), static_cast<charT>(0));
		// Don't include null terminator (result of .end())
		std::span<const charT> stringSpan(m_GLSLString.cbegin(), --m_GLSLString.cend());

		if (file.Read(stringSpan) != m_GLSLString.length()) 
			throw std::runtime_error("Failed to read file");

		ParseVersionString();
	}

	template<typename C, typename CTr>
	requires (std::is_convertible_v<C, charT> &&
		std::is_same_v<Tr, CTr>)
	void Parse(std::basic_string_view<C, CTr> view) {
		m_GLSLString = view;
		ParseVersionString();
	}

	template<typename C, std::size_t Extent = std::dynamic_extent>
	requires std::is_convertible_v<C, charT>
	void Parse(std::span<C, Extent> span) {
		m_GLSLString = string_type(span.begin(), span.end());
		ParseVersionString();
	}

	// Free the memory held onto by string
	void Free() noexcept {
		string_view_type().swap(m_VersionString);
		string_type().swap(m_GLSLString);
	}

	const string_type& String() const { return m_GLSLString; }
	const charT *CString() const { return m_GLSLString.c_str(); }
	string_view_type VersionString() const { return m_VersionString; }
	int VersionNum() const { return m_VersionNum; }

	std::size_t Length() const { return m_GLSLString.length(); }

protected:
	string_view_type ParseVersionString() {
		auto startOfLine = 0;
		auto endOfLine = m_GLSLString.npos;
		auto endOfGLSL = m_GLSLString.npos;
		for (;;) {
			endOfLine = m_GLSLString.find_first_of('\n', startOfLine);
			string_view_type line(std::begin(m_GLSLString) + startOfLine,
					  std::begin(m_GLSLString) + endOfLine);

			/*
			 * Instead of setting flags and going into indentation hell consider using
			 * continue instead. This choice wasn't made initially becuase the final
			 * lines in this loop are what increment the counter and allow breaking
			 * from the loop
			 */
			auto poundPos = line.find_first_of('#');
			if (poundPos != line.npos) {
				string_view_type beforePound = line.substr(0, poundPos);
				bool isPreprocessorLine = true;
				for (auto c : beforePound) {
					if (std::isblank(c) != 0)
						isPreprocessorLine = false;
				}
				if (isPreprocessorLine) {
					string_view_type directive = "version";
					auto startofDirective = line.find(directive, poundPos);
					if (startofDirective != line.npos) {
						string_view_type num = line.substr(startofDirective + directive.length());
						bool validNum = true;
						for (auto c : num) {
							if ((std::isblank(c) != 0) && (std::isdigit(c) != 0))
								validNum = false;
						}
						if (validNum) {
							// Temporary construct string for null terminator
							m_VersionNum = std::stoi(string_type(num.data(), num.length()));
							if (m_VersionNum <= 0) {
								std::string msg = "GLSL version directive invalid: ";
								msg.append(line, line.length() - 1); // remove \n
								throw std::runtime_error(msg);
							}
							return (m_VersionString = line);
						}
					}
				}
			}

			if (endOfLine == m_GLSLString.npos)
				break;
			startOfLine = endOfLine + 1;
		} 
		return (m_VersionString = string_view_type());
	}

private:
	string_type m_GLSLString;
	string_view_type m_VersionString;
	int m_VersionNum;
};

/*
 **************************************************
 **************** Helper Functions ****************
 **************************************************
 */
template<typename charT, typename Tr, typename Alloc>
void swap(GLSLParser<charT, Tr, Alloc>& lhs, GLSLParser<charT, Tr, Alloc>& rhs)
{
	lhs.Swap(rhs);
}
}

#endif

