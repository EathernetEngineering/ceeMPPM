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

#include <cee/core/files.h>

#include <fmt/format.h>

#include <endian.h>

namespace cee {
namespace files {
	BitmapFile::BitmapFile(const std::string &fileName, FileMode mode)
	 : m_FileName(fileName) {
		std::ios_base::openmode m{};
		if ((mode & FileMode::In) == FileMode::In) m |= std::ios::in;
		if ((mode & FileMode::Out) == FileMode::Out) m |= std::ios::out;
		m_Fstream.open(m_FileName, m | std::ios::binary);
		if (!m_Fstream.is_open()) {
			throw FileError(m_FileName, "Failed to open file {}");
		}
	}

	BitmapFile::~BitmapFile() {
	}

	void BitmapFile::Read() {
		ReadHeaders();
		ReadPixels();
	}

	const std::vector<uint8_t> &BitmapFile::Read(BitmapInfo &info) {
		Read();
		info = GetInfo();
		return GetPixels();
	}

	void BitmapFile::Write(const BitmapInfo& info, const std::vector<uint8_t> &data) {
		{
			if (info.size != data.size()) {
				std::string msg = fmt::format("BitmapFile::Write(): "
						"info.size (which is {}) != data.size() (which is {})",
						m_FileName, info.size, data.size());
				throw FileError(m_FileName, msg);
			}
			if (info.bitsPerPixel != 24 && info.bitsPerPixel != 32) {
				std::string msg = fmt::format("BitmapFile::Write(): "
						"Only 24 and 32bpp supported (requested: {}bpp)",
						info.bitsPerPixel);
				throw FileError(m_FileName, msg);
			}
			int64_t width = info.width;
			int64_t height = info.height;
			int64_t bpp = info.bitsPerPixel;
			if (((width * height * bpp) / 8ul) != info.size) {
				std::string msg = "BitmapFile::Write(): Invalid file size";
#ifndef NDEBUG
				msg += fmt::format("\treason: ((width (which is {}) * height (which is {}) * "
						"bpp (which is {})) / 8) != imageSize (which is {})",
						width, height, bpp, uint32_t(m_InfoHeader.imageSize));
#endif
				throw FileError(m_FileName, msg);
		}
		}
		m_FileHeader.magic[0] = 0x42;
		m_FileHeader.magic[1] = 0x4D;
		m_FileHeader.fileSize = sizeof(m_FileHeader) + sizeof(m_InfoHeader) + data.size();
		m_FileHeader.reserved0 = 0;
		m_FileHeader.reserved1 = 0;
		m_FileHeader.pixelOffset = sizeof(m_FileHeader) + sizeof(m_InfoHeader);

		m_InfoHeader.infoSize = sizeof(m_InfoHeader);
		m_InfoHeader.width = info.width;
		m_InfoHeader.height = info.height;
		m_InfoHeader.colorPlaneCount = 1;
		m_InfoHeader.bitsPerPixel = info.bitsPerPixel;
		m_InfoHeader.compressionMethod = 0;
		m_InfoHeader.imageSize = info.size;
		m_InfoHeader.xPixelsPerM = info.xPixelsPerM;
		m_InfoHeader.yPixelsPerM = info.yPixelsPerM;
		m_InfoHeader.colorsUsed = 0;
		m_InfoHeader.colorsImportant = 0;

		m_Pixels = data;

		m_Fstream.seekg(0);
		m_Fstream.write(reinterpret_cast<const char *>(&m_FileHeader), sizeof(m_FileHeader));
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}
		m_Fstream.write(reinterpret_cast<const char *>(&m_InfoHeader), sizeof(m_InfoHeader));
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}
		m_Fstream.write(reinterpret_cast<const char *>(m_Pixels.data()), m_Pixels.size());
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}
	}

	BitmapInfo BitmapFile::GetInfo() const {
		return {
			.width = m_InfoHeader.width,
			.height = m_InfoHeader.height,
			.bitsPerPixel = m_InfoHeader.bitsPerPixel,
			.size = m_InfoHeader.imageSize,
			.xPixelsPerM = m_InfoHeader.xPixelsPerM,
			.yPixelsPerM = m_InfoHeader.yPixelsPerM
		};
	}

	void BitmapFile::ReadHeaders() {
		uint32_t realFileSize;
		uint32_t infoHeaderSize;

		m_Fstream.seekg(0, std::ios::end);
		realFileSize = m_Fstream.tellg();
		m_Fstream.seekg(0, std::ios::beg);
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}
		if (realFileSize < sizeof(Header)) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}

		m_Fstream.read(reinterpret_cast<char *>(&m_FileHeader), sizeof(m_FileHeader));
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}

		if (m_FileHeader.magic[0] != 0x42 || m_FileHeader.magic[1] != 0x4D) {
			throw FileError(m_FileName, "BitmapFile::Write(): File error");
		}

		m_FileHeader.fileSize = le32toh(m_FileHeader.fileSize);
		m_FileHeader.pixelOffset = le32toh(m_FileHeader.pixelOffset);

		if (m_FileHeader.fileSize != realFileSize) {
			std::string msg = "BitmapFile::Read(): Invalid file size";
#ifndef NDEBUG
			msg += fmt::format("\treason: fileSize (which is {}) != realFileSize (which is {})",
					uint32_t(m_FileHeader.fileSize), realFileSize);
#endif
			throw FileError(m_FileName, msg);
		}
		if (m_FileHeader.pixelOffset > m_FileHeader.fileSize) {
			std::string msg = "BitmapFile::Read(): Invalid file size";
#ifndef NDEBUG
			msg += fmt::format("\treason: pixelOffset (which is {}) > fileSize (which is {})",
					uint32_t(m_FileHeader.pixelOffset), uint32_t(m_FileHeader.fileSize));
#endif
			throw FileError(m_FileName, msg);
		}

		m_Fstream.read(reinterpret_cast<char *>(&infoHeaderSize), sizeof(infoHeaderSize));
		if (!m_Fstream) {
			std::string msg = "BitmapFile::Read(): File error";
			throw FileError(m_FileName, msg);
		}

		infoHeaderSize = std::min(le32toh(infoHeaderSize), static_cast<uint32_t>(sizeof(m_InfoHeader)));
		if (infoHeaderSize < 12 || infoHeaderSize > 124) {
			std::string msg = "BitmapFile::Read(): Invalid file size";
#ifndef NDEBUG
			msg += fmt::format("\treason: infoHeaderSize (which is {}) < 12 || infoHeaderSize (which is {}) > 124",
					infoHeaderSize, infoHeaderSize);
#endif
			throw FileError(m_FileName, msg);
		}

		memset(&m_InfoHeader, 0, sizeof(m_InfoHeader));
		m_Fstream.seekg(-(static_cast<int64_t>(sizeof(infoHeaderSize))), std::ios::cur);
		m_Fstream.read(reinterpret_cast<char *>(&m_InfoHeader), infoHeaderSize);
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Read(): File error");
		}

		m_InfoHeader.infoSize = infoHeaderSize;
		m_InfoHeader.width = le32toh(m_InfoHeader.width);
		m_InfoHeader.height = le32toh(m_InfoHeader.height);
		m_InfoHeader.colorPlaneCount = le16toh(m_InfoHeader.colorPlaneCount);
		m_InfoHeader.bitsPerPixel = le16toh(m_InfoHeader.bitsPerPixel);
		m_InfoHeader.compressionMethod = le32toh(m_InfoHeader.compressionMethod);
		m_InfoHeader.imageSize = le32toh(m_InfoHeader.imageSize);
		m_InfoHeader.xPixelsPerM = le32toh(m_InfoHeader.xPixelsPerM);
		m_InfoHeader.yPixelsPerM = le32toh(m_InfoHeader.xPixelsPerM);
		m_InfoHeader.colorsUsed = le32toh(m_InfoHeader.colorsUsed);
		m_InfoHeader.colorsImportant = le32toh(m_InfoHeader.colorsImportant);

		int64_t width  = static_cast<int64_t>(m_InfoHeader.width);
		int64_t height = static_cast<int64_t>(m_InfoHeader.height);
		int64_t bpp    = static_cast<int64_t>(m_InfoHeader.bitsPerPixel);
		if (m_InfoHeader.imageSize > m_FileHeader.fileSize) {
			std::string msg = "BitmapFile::Read(): Invalid file size";
#ifndef NDEBUG
			msg += fmt::format("\treason: fileSize (which is {}) < imageSize (which is {}))",
					uint32_t(m_FileHeader.fileSize), uint32_t(m_InfoHeader.imageSize));
#endif
			throw FileError(m_FileName, msg);
		}
		if (((width * height * bpp) / 8ul) != m_InfoHeader.imageSize) {
			std::string msg = "BitmapFile::Read(): Invalid file size";
#ifndef NDEBUG
			msg += fmt::format("\treason: ((width (which is {}) * height (which is {}) * "
					"bpp (which is {})) / 8) != imageSize (which is {})",
					width, height, bpp, uint32_t(m_InfoHeader.imageSize));
#endif
			throw FileError(m_FileName, msg);
		}
	}

	void BitmapFile::ReadPixels() {
		m_Pixels.resize(m_InfoHeader.imageSize);
		m_Fstream.seekg(m_FileHeader.pixelOffset);
		m_Fstream.read(reinterpret_cast<char *>(m_Pixels.data()), m_Pixels.size());
		if (!m_Fstream) {
			throw FileError(m_FileName, "BitmapFile::Read(): File error");
		}
	}
}
}


