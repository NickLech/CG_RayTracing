#include <GLContext.hpp>
#include <GpuBuffer.hpp>
#include <Macros.hpp>

#include <expected>
#include <optional>

namespace cg_raytracing {
	class IndexBuffer {
	public :
		IndexBuffer(IndexBuffer&& _prev) noexcept;
		~IndexBuffer();

		IndexBuffer& operator=(IndexBuffer&& _other) noexcept;

		/// <summary>
		/// Create a new index buffer with enough capacity to
		/// hold _size indexes
		/// </summary>
		/// <param name="_size">Max. number of indexes</param>
		/// <returns>Buffer or error</returns>
		static std::expected<IndexBuffer, GLError> CreateIndexBuffer(size_t _size);

		/// <summary>
		/// Push single index
		/// </summary>
		/// <param name="_index"The index></param>
		/// <returns>Error if buffer overflow</returns>
		std::optional<GLError> PushIndex(uint32_t _index);

		std::optional<GLError> PushIndexes(std::vector<uint32_t> const& _indexes);

		/// <summary>
		/// Push index data directly from a pointer, _size
		/// indexes are copied
		/// </summary>
		/// <param name="_data">Pointer to index data</param>
		/// <param name="_size">Number of indexes</param>
		/// <returns>Possible error</returns>
		std::optional<GLError> PushIndexesUnsafe(uint32_t const* _data, size_t _size);

		/// <summary>
		/// Get current used buffer size (in number
		/// of indices)
		/// </summary>
		/// <returns></returns>
		FORCE_INLINE size_t GetBufferSize() const {
			return m_curr_size;
		}

		/// <summary>
		/// Get internal buffer capacity (in number
		/// of indices)
		/// </summary>
		/// <returns></returns>
		FORCE_INLINE size_t GetBufferCapacity() const {
			return m_buf.GetBufferSize() / sizeof(uint32_t);
		}

		/// <summary>
		/// Reset the used buffer size
		/// to zero
		/// </summary>
		/// <returns></returns>
		FORCE_INLINE void ClearBuffer() {
			m_curr_size = 0;
		}

		void Bind() const;

		/// <summary>
		/// Clone this index buffer
		/// </summary>
		/// <returns>Clone or error</returns>
		std::expected<IndexBuffer, GLError> Clone() const;

	private :
		IndexBuffer();
		IndexBuffer(IndexBuffer const& _other) = delete;

	private :
		GpuBuffer m_buf;
		size_t    m_curr_size;
	};
}