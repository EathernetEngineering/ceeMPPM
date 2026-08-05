/*
 * ceeCore
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

#ifndef CEE_CORE_FILE_MANAGER_H_
#define CEE_CORE_FILE_MANAGER_H_

#include <cee/core/except.h>

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace cee {
namespace files {
	/*
	 **************************************************
	 **************** Exception types *****************
	 **************************************************
	 */
	class FileError : core::Error {
	public:
		explicit FileError(const std::string &file, const std::string &what)
		 : Error(what), m_FileName(file) {
		}
		explicit FileError(const std::string &file, const char *what)
		 : Error(what), m_FileName(file) {
		}
		FileError(const FileError &other) = default;
		FileError &operator=(const FileError &other) = default;

		const std::string &GetFileName() const noexcept { return m_FileName; }

	private:
		std::string m_FileName;
	};

	/*
	 **************************************************
	 ************** File behaviour flags **************
	 ********** Associated bitwise operators **********
	 **************************************************
	 */
	enum class FileMode : int {
		None = 0,
		In = (1 << 0),
		Out = (1 << 1),
		InOut = In | Out,
	};

	constexpr FileMode operator&(FileMode lhs, FileMode rhs) noexcept
	{
		return static_cast<FileMode>(static_cast<int>(lhs) &
												static_cast<int>(rhs));
	}
	constexpr FileMode& operator&=(FileMode& lhs, FileMode rhs) noexcept
	{
		return (lhs = lhs & rhs);
	}
	constexpr FileMode operator|(FileMode lhs, FileMode rhs) noexcept
	{
		return static_cast<FileMode>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
	constexpr FileMode& operator|=(FileMode& lhs,  FileMode rhs) noexcept
	{
		return (lhs = lhs | rhs);
	}
	constexpr FileMode operator^(FileMode lhs, FileMode rhs) noexcept
	{
		return static_cast<FileMode>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
	}
	constexpr FileMode& operator^=(FileMode& lhs, FileMode rhs) noexcept
	{
		return (lhs = lhs ^ rhs);
	}

	/*
	 **************************************************
	 ********************* Bitmap *********************
	 **************************************************
	 */

	struct BitmapInfo {
		uint32_t width;
		uint32_t height;
		uint16_t bitsPerPixel;
		uint32_t size;
		uint32_t xPixelsPerM;
		uint32_t yPixelsPerM;
	};

	class BitmapFile {
	private:
		struct Header {
			char magic[2];
			uint32_t fileSize;
			uint16_t reserved0;
			uint16_t reserved1;
			uint32_t pixelOffset;
		} __attribute__((packed));

		struct InfoHeader {
			uint32_t infoSize;
			uint32_t width;
			uint32_t height;
			uint16_t colorPlaneCount;
			uint16_t bitsPerPixel;
			uint32_t compressionMethod;
			uint32_t imageSize;
			uint32_t xPixelsPerM;
			uint32_t yPixelsPerM;
			uint32_t colorsUsed;
			uint32_t colorsImportant;
		} __attribute__((packed));

	public:
		BitmapFile(const std::string &m_FileName, FileMode mode = FileMode::InOut);
		~BitmapFile();

		void Read();
		const std::vector<uint8_t> &Read(BitmapInfo &info);
		void Write(const BitmapInfo& info, const std::vector<uint8_t> &data);

		BitmapInfo GetInfo() const;
		const std::vector<uint8_t> &GetPixels() const { return m_Pixels; }

	private:
		void ReadHeaders();
		void ReadPixels();

	private:
		std::string m_FileName;
		std::fstream m_Fstream;
		Header m_FileHeader;
		InfoHeader m_InfoHeader;
		std::vector<uint8_t> m_Pixels;
	};
}
}

#endif

