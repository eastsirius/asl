/**
 * @file memory.cpp
 * @brief 内存工具
 * @author 程行通
 */

#include "memory.hpp"

namespace ASL_NAMESPACE {
	Buffer::Buffer() : m_nDataSize(0), m_nBufSize(0), m_pBuffer(NULL) {
	}

	Buffer::Buffer(size_t size) {
		Buffer();
		Create(size);
	}

	Buffer::~Buffer() {
		Release();
	}

	bool Buffer::Create(size_t size) {
		Release();
		return Recreate(size);
	}

	bool Buffer::Recreate(size_t new_size) {
		if(m_nDataSize > new_size) {
			return false;
		}

		uint8_t* pNewBuf = (uint8_t*)realloc(m_pBuffer, new_size);
		if(!pNewBuf) {
			return false;
		}
		m_pBuffer = pNewBuf;
		m_nBufSize = new_size;

		return true;
	}

	void Buffer::Release() {
		m_nDataSize = 0;
		m_nBufSize = 0;
		if(m_pBuffer) {
			free(m_pBuffer);
			m_pBuffer = NULL;
		}
	}

	bool Buffer::AppendData(size_t size) {
		return AppendData(NULL, size);
	}

	bool Buffer::AppendData(const uint8_t* data, size_t size) {
		if(size > GetFreeSize()) {
			return false;
		}
		if(data) {
			memcpy(GetBuffer(GetDataSize()), data, size);
		}
		m_nDataSize += size;
		return true;
	}

	void Buffer::SkipData(size_t size) {
		size_t s = asl_min(m_nDataSize, size);
		m_nDataSize -= s;
		if(m_nDataSize > 0) {
			memmove(m_pBuffer, m_pBuffer + s, m_nDataSize);
		}
	}


	GrowthBuffer::GrowthBuffer() : Buffer() {
	}

	GrowthBuffer::GrowthBuffer(size_t size) : Buffer(size) {
	}

	GrowthBuffer::~GrowthBuffer() {
		Release();
	}

	bool GrowthBuffer::RequestFreeSize(size_t size) {
		if(GetFreeSize() >= size) {
			return true;
		}

		size_t nNewDataSize = GetDataSize() + size;
		size_t nNewSize = GetBufferSize();
		if(nNewSize == 0) {
			nNewSize = 1024;
		}
		while(nNewSize < nNewDataSize) {
			nNewSize *= 2;
		}

		return Recreate(nNewSize);
	}
}
