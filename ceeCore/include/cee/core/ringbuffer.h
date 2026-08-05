/*
 * ceeCore
 * Copyright (C) 2025 2026 Chloe Eather
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

#include <cee/core/except.h>

#include <array>

namespace cee {
	template<typename T, std::size_t N>
	class RingBuffer {
	public:
		RingBuffer()
		 : m_ReadOffset(0), m_WriteOffset(0), m_Size(0) {
		}

		RingBuffer(const RingBuffer &other) {
			*this = other;
		}

		RingBuffer(RingBuffer &&other) {
			*this = std::move(other);
		}

		RingBuffer &operator=(const RingBuffer &other) {
			m_Buffer = other.m_Buffer;
			m_ReadOffset = other.m_ReadOffset;
			m_WriteOffset = other.m_WriteOffset;
			m_Size = other.m_Size;
		}

		RingBuffer &operator=(RingBuffer &&other) {
			m_Buffer = std::move(other.m_Buffer);
			m_ReadOffset = other.m_ReadOffset;
			m_WriteOffset = other.m_WriteOffset;
			m_Size = other.m_Size;
		}

		bool Empty() const { return m_Size == 0; }
		bool Full() const { return m_Size == N; }
		std::size_t Size() const { return m_Size; }

		void Enqueue(const T &val) {
			if (Full())
				throw core::UsageError("RingBuffer::Enqueue(): Buffer full");
			m_Buffer[m_WriteOffset] = val;
			m_WriteOffset = (m_WriteOffset + 1) % N;
			++m_Size;
		}

		void Enqueue(T &&val) {
			if (Full())
				throw core::UsageError("RingBuffer::Enqueue(): Buffer full");
			m_Buffer[m_WriteOffset] = std::move(val);
			m_WriteOffset = (m_WriteOffset + 1) % N;
			++m_Size;
		}

		T Dequeue() {
			if (Empty())
				throw core::UsageError("RingBuffer::Dequeue(): Buffer empty");
			size_t offset = m_ReadOffset;
			m_ReadOffset = (m_ReadOffset + 1) % N;
			--m_Size;

			return std::move(m_Buffer[offset]);
		}

		const T &Peek() const {
			if (Empty())
				throw core::UsageError("RingBuffer::Peek(): Buffer empty");
			return m_Buffer[m_ReadOffset];
		}

		T &Peek() {
			if (Empty())
				throw core::UsageError("RingBuffer::Peek(): Buffer empty");
			return m_Buffer[m_ReadOffset];
		}

		RingBuffer &Swap(RingBuffer &other) {
			m_Buffer.swap(other.m_Buffer);
			std::swap(m_ReadOffset, other.m_ReadOffset);
			std::swap(m_WriteOffset, other.m_WriteOffset);
			std::swap(m_Size, other.m_Size);

			return *this;
		}

	private:
		std::array<T, N> m_Buffer;
		std::size_t m_ReadOffset;
		std::size_t m_WriteOffset;
		std::size_t m_Size;
	};
}

