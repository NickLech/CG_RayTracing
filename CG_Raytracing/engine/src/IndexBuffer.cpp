#include <IndexBuffer.hpp>

#include <GL/glew.h>

namespace cg_raytracing {
	IndexBuffer::IndexBuffer() :
		m_buf{},
		m_curr_size{} 
	{}

	IndexBuffer::IndexBuffer(IndexBuffer&& _prev) noexcept :
		m_buf{std::move(_prev.m_buf)}, 
		m_curr_size{_prev.m_curr_size}
	{}

	IndexBuffer::~IndexBuffer()
	{}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& _other) noexcept {
		m_buf = std::move(_other.m_buf);
		m_curr_size = _other.m_curr_size;
		return *this;
	}

	std::expected<IndexBuffer, GLError> IndexBuffer::CreateIndexBuffer(size_t _size) {
		auto buf_temp = GpuBuffer::CreateBuffer(_size * sizeof(uint32_t),
			BufferMapping::WRITE |
			BufferMapping::PERSISTENT |
			BufferMapping::COHERENT |
			BufferMapping::DYNAMIC
		);
		if (!buf_temp.has_value()) {
			return std::unexpected{ buf_temp.error() };
		}
		auto index_buf = IndexBuffer();
		index_buf.m_buf = std::move(buf_temp.value());

		auto maybe_err = index_buf.m_buf.MapBuffer(0, index_buf.m_buf.GetBufferSize(),
			BufferMapping::WRITE |
			BufferMapping::PERSISTENT |
			BufferMapping::COHERENT
		);

		if (maybe_err.has_value()) {
			return std::unexpected{ maybe_err.value() };
		}
		return index_buf;
	}

	std::optional<GLError> IndexBuffer::PushIndex(uint32_t _index) {
		return PushIndexesUnsafe(&_index, 1);
	}

	std::optional<GLError> IndexBuffer::PushIndexes(std::vector<uint32_t> const& _indexes) {
		return PushIndexesUnsafe(_indexes.data(), _indexes.size());
	}

	std::optional<GLError> IndexBuffer::PushIndexesUnsafe(uint32_t const* _data, size_t _size) {
		auto curr_byte_offset = m_curr_size * sizeof(uint32_t);
		auto copy_size = (_size * sizeof(uint32_t));
		auto last_byte_offset = curr_byte_offset + copy_size;
		if (m_buf.GetBufferSize() < last_byte_offset) {
			return GLError::PUSH_FAILED_BUFFER_OVERFLOW;
		}
		auto maybe_ptr = m_buf.GetHostBuffer();
		auto buf_ptr = maybe_ptr.value();
		std::copy_n((const uint8_t*)_data, copy_size, (uint8_t*)(buf_ptr)+curr_byte_offset);
		m_curr_size += _size;
		return std::nullopt;
	}

	void IndexBuffer::Bind() const {
		m_buf.BindBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}
}