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

		static std::expected<IndexBuffer, GLError> CreateIndexBuffer(size_t _size);

		std::optional<GLError> PushIndex(uint32_t _index);

		std::optional<GLError> PushIndexes(std::vector<uint32_t> const& _indexes);

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

	private :
		IndexBuffer();
		IndexBuffer(IndexBuffer const& _other) = delete;

	private :
		GpuBuffer m_buf;
		size_t    m_curr_size;
	};
}