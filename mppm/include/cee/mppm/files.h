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

#ifndef CEE_FILE_MANAGER_H_
#define CEE_FILE_MANAGER_H_

#include <cee/mppm/assertions.h>
#include <cee/mppm/util.h>

#include <filesystem>
#include <span>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>

namespace cee {
namespace files {
	/*
	 **************************************************
	 ************** File behaviour flags **************
	 ********** Associated bitwise operators **********
	 **************************************************
	 */
	enum class FileMode {
		None = 0,
		In = (1 << 0),
		Out = (1 << 1),
		InOut = In | Out,
		Create = (1 << 2),
		Truncate = (1 << 3),
	};

	constexpr FileMode operator&(FileMode lhs, FileMode rhs) noexcept
	{
		return static_cast<FileMode>(util::ToUnderlyingType(lhs) &
												util::ToUnderlyingType(rhs));
	}
	constexpr FileMode& operator&=(FileMode& lhs, FileMode rhs) noexcept
	{
		return (lhs = lhs & rhs);
	}
	constexpr FileMode operator|(FileMode lhs, FileMode rhs) noexcept
	{
		return static_cast<FileMode>(util::ToUnderlyingType(lhs) | util::ToUnderlyingType(rhs));
	}
	constexpr FileMode& operator|=(FileMode& lhs,  FileMode rhs) noexcept
	{
		return (lhs = lhs | rhs);
	}
	constexpr FileMode operator^(FileMode lhs, FileMode rhs) noexcept
	{
		return static_cast<FileMode>(util::ToUnderlyingType(lhs) ^ util::ToUnderlyingType(rhs));
	}
	constexpr FileMode& operator^=(FileMode& lhs, FileMode rhs) noexcept
	{
		return (lhs = lhs ^ rhs);
	}
	constexpr FileMode operator>>(FileMode lhs, int rhs) noexcept
	{
		return static_cast<FileMode>(util::ToUnderlyingType(lhs) >> rhs);
	}
	constexpr FileMode& operator>>=(FileMode& lhs, int rhs) noexcept
	{
		return (lhs = lhs >> rhs);
	}
	constexpr FileMode operator<<(FileMode lhs, int rhs) noexcept
	{
		return static_cast<FileMode>(util::ToUnderlyingType(lhs) << rhs);
	}
	constexpr FileMode& operator<<=(FileMode& lhs, int rhs) noexcept
	{
		return (lhs = lhs << rhs);
	}
	constexpr FileMode operator~(FileMode lhs) noexcept
	{
		return static_cast<FileMode>(~util::ToUnderlyingType(lhs));
	}

	/*
	 **************************************************
	 **************** Generic File Type ***************
	 **************************************************
	 */
	enum class SeekMode : int8_t {
		None = -1,
		Offset = 0,
		Absolute = 1,
		End = 2,
		Hole = 3,
		Data = 4
	};

	template<typename T, typename Tr = std::char_traits<T>>
	requires std::is_integral_v<T>
	class BasicFile {
	public:
		using size_type = ssize_t;
		using pos_type = Tr::pos_type;
		using off_type = Tr::off_type;

	public:
		// Base constructor/destructor
		explicit BasicFile(std::filesystem::path path, FileMode f = FileMode::InOut, int m = 0)
		 : m_Path(std::move(path)), m_Mode(f), m_Access(m), m_Fd(0), m_Size(0),
		   m_CursorPosition(0)
		{
			int openFlags;
			if (!std::filesystem::exists(m_Path) && ((m_Mode & FileMode::Create) == FileMode::None))
				throw std::runtime_error("Attempting to open file that doesn't exist");
			if (std::filesystem::is_directory(m_Path))
				throw std::runtime_error("Attempting to open directory as file");

			openFlags = ConstructOpenFlags(m_Mode);
			if ((m_Mode & FileMode::Create) != FileMode::None) {
				m_Fd = open(m_Path.c_str(), openFlags, m_Access);
			} else {
				m_Fd = open(m_Path.c_str(), openFlags);
			}
			if (m_Fd < 0) {
				throw std::system_error(std::error_code(errno, std::system_category()));
			}

			m_Size = lseek(m_Fd, 0, SEEK_END);
			if (m_Size < 0)
				throw std::system_error(std::error_code(errno, std::system_category()));

			m_CursorPosition = lseek(m_Fd, 0, SEEK_SET);
			if (m_CursorPosition < 0)
				throw std::system_error(std::error_code(errno, std::system_category()));

			struct stat statData;
			int returnVal = fstat(m_Fd, &statData);
			if (returnVal > 0)
				throw std::system_error(std::error_code(errno, std::system_category()));
			if (m_Size != statData.st_size)
				throw std::runtime_error("Expected calculated file size to be same as stat()");
			if (m_Access != statData.st_mode) {
				m_Access = statData.st_mode;
				CEE_ASSERT(m_Mode != FileMode::Create, "st_mode does not match newly created file.");
			}
		}

		~BasicFile() noexcept {
			if (m_Fd > 0) {
				close(m_Fd);
			}
		}

		// Copy constructor/assignment operator (deleted)
		BasicFile(const BasicFile& other) = delete;
		BasicFile& operator=(const BasicFile& other) = delete;

		// Copy constructor/assignment operator
		BasicFile(BasicFile&& other) noexcept
		 : m_Path(std::move(other.m_Path)), m_Mode(other.m_Mode), m_Access(other.m_Access),
		   m_Fd(other.m_Fd), m_Size(other.m_Size), m_CursorPosition(other.m_CursorPosition)
		{
			other.m_Path.clear();
			m_Mode = FileMode::None;
			m_Access = 0;
			m_Fd = -1;
			m_Size = 0;
			m_CursorPosition = 0;
		}

		BasicFile& operator=(BasicFile&& other) noexcept
		{
			m_Path = std::move(other.m_Path);
			other.m_Path.clear();
			m_Mode = std::exchange(other.m_Mode, FileMode::None);
			m_Access = std::exchange(other.m_Access, 0);
			m_Fd = std::exchange(other.m_Fd, -1);
			m_Size = std::exchange(other.m_Size, 0);
			m_CursorPosition = std::exchange(other.m_CursorPosition, 0);
		}

		// Swap operation
		void Swap(BasicFile& other) noexcept {
			m_Path.swap(other.m_Path);
			m_Mode = std::exchange(other.m_Mode, m_Mode);
			m_Access = std::exchange(other.m_Access, m_Access);
			m_Fd = std::exchange(other.m_Fd, m_Fd);
			m_Size = std::exchange(other.m_Size, m_Size);
			m_CursorPosition = std::exchange(other.m_CursorPosition, m_CursorPosition);
		}

		// Read to a span of bytes from file
		template<typename SpanT, std::size_t Extent = std::dynamic_extent>
		requires (std::is_convertible_v<SpanT, T> && sizeof(T) == sizeof(SpanT))
		size_type Read(std::span<SpanT, Extent> data) {
			return Read(data.data(), data.size());
		}

		// Appends `length` bytes to  the vector from file.
		template<typename VecT, typename Alloc>
		requires (std::is_convertible_v<VecT, T> && (sizeof(T) == sizeof(VecT)))
		size_type Read(std::vector<VecT, Alloc>& data, size_type bytes) {
			CEE_ASSERT((bytes % sizeof(T) == 0), "This will surely lead to an unaligned read");

			std::vector<VecT, Alloc> scratch(bytes/sizeof(VecT), (VecT)0);
			size_type bytesRead = Read(scratch.data(), scratch.size() * sizeof(VecT));
			data.insert(data.end(), scratch.begin(), scratch.end());

			return bytesRead;
		}

		// Write a span of bytes to file
		template<typename SpanT, std::size_t Extent = std::dynamic_extent>
		requires (std::is_convertible_v<SpanT, T> && (sizeof(T) == sizeof(SpanT)))
		size_type Write(std::span<SpanT, Extent> data) {
			return Write(data.data(), data.size());
		}

		// Write an entire vector to file
		template<typename VecT, typename Alloc>
		requires (std::is_convertible_v<VecT, T> && (sizeof(T) == sizeof(VecT)))
		size_type Write(const std::vector<VecT, Alloc>& data) {
			return Write(data.data(), data.size());
		}

		// Getters and setters
		inline const std::filesystem::path& GetPath() const noexcept { return m_Path; }
		inline size_type GetSize() const noexcept { return m_Size; }
		inline off_type GetCurrentPos() const noexcept { return m_CursorPosition; }

		off_type SetCurrentPos(off_type offset, SeekMode m) {
			int whence, result;
			whence = SetPosToCFlag(m);
			CEE_ASSERT(whence != -1, "Invalid SetCurrentPos flags given");
			m_CursorPosition = lseek(m_Fd, offset, whence);
			if (m_CursorPosition < 0)
				throw std::system_error(std::error_code(errno, std::system_category()));
			return m_CursorPosition;
		}

	private:
		static constexpr int SetPosToCFlag(SeekMode m) noexcept {
			switch (m) {
				case SeekMode::Offset:
					return SEEK_CUR;

				case SeekMode::Absolute:
					return SEEK_SET;

				case SeekMode::End:
					return SEEK_END;

				case SeekMode::Hole:
					return SEEK_HOLE;

				case SeekMode::Data:
					return SEEK_DATA;

				default:
					return -1;
			}
		}

		static constexpr int ConstructOpenFlags(FileMode m) noexcept {
			int f = 0;
			switch ((m & FileMode::InOut)) {
				case FileMode::InOut:
					f |= O_RDWR;
					break;

				case FileMode::In:
					f |= O_RDONLY;
					break;

				case FileMode::Out:
					f |= O_WRONLY;
					break;

				default:
					(void)0; // Supress compiler warning
			}
			if ((m & FileMode::Create) != FileMode::None)
				f |= O_CREAT;
			if ((m & FileMode::Truncate) != FileMode::None)
				f |= O_TRUNC;
			
			return f;
		}

		// Unsafe c style read and write, only call using safe array wrappers
		size_type Read(T *data, const size_type length) {
			size_type bytesRead = 0, totalBytesRead = 0;
			do {
				bytesRead = read(m_Fd,
					 (void *)((char *)data + totalBytesRead),
					 length - totalBytesRead);
				if (bytesRead < 0) {
					if (errno == EAGAIN) // this is fine, don't throw
						continue;
					throw std::system_error(std::error_code(errno, std::system_category()));
				} else if (bytesRead == 0) {
					size_type readRemainder = totalBytesRead % sizeof(T);
					CEE_ASSERT(readRemainder == 0,
						"Alignment error: Type to read {} has size {} and {} bytes were read. "
						"{} bytes missing for alignment, they have been zero-initialized.",
						typeid(T).name(), sizeof(T), totalBytesRead, readRemainder);

					break;
				}
				totalBytesRead += bytesRead;
			} while (length > totalBytesRead);

			SetCurrentPos(0, SeekMode::Offset);

			return totalBytesRead;
		}

		size_type Write(const T *data, const size_type length) {
			size_type bytesWritten = 0, totalBytesWritten = 0;
			do {
				bytesWritten = write(m_Fd,
						 (void *)((char *)data + totalBytesWritten),
						 length - totalBytesWritten);
				if (bytesWritten < 0) {
					if (errno == EAGAIN) // this is fine, don't throw
						continue;
					throw std::system_error(std::error_code(errno, std::system_category()));
				} else if (bytesWritten == 0) {
					CEE_ASSERT(((totalBytesWritten % sizeof(T)) == 0),
						"Alignment error: Type to write {} has size {} and {} bytes were written. "
						"{} bytes missing for alignment, they have been zero-initialized.",
						typeid(T).name(), sizeof(T), totalBytesWritten, totalBytesWritten % sizeof(T));
					break;
				}
				totalBytesWritten += bytesWritten;
			} while (length > totalBytesWritten);

			off_type oldCursorPos = SetCurrentPos(0, SeekMode::Offset);
			m_Size = SetCurrentPos(0, SeekMode::End);
			SetCurrentPos(oldCursorPos, SeekMode::Absolute);

			return totalBytesWritten;
		}

	private:
		std::filesystem::path m_Path;
		FileMode m_Mode;
		int m_Access;
		int m_Fd;
		size_type m_Size;
		off_type m_CursorPosition;
	};

	// Alias
	using File = BasicFile<char, std::char_traits<char>>;

	/*
	 **************************************************
	 **************** Helper Functions ****************
	 **************************************************
	 */
	template<typename T, typename Tr>
	void swap(BasicFile<T, Tr>& lhs, BasicFile<T, Tr>& rhs)
	{
		lhs.Swap(rhs);
	}
}
}

#endif

