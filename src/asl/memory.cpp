/**
 * @file memory.cpp
 * @brief 内存工具
 * @author 程行通
 */

#include "memory.hpp"

namespace ASL_NAMESPACE {
	Buffer::Buffer() : m_nDataSize(0), m_nBufSize(0), m_pBuffer(NULL) {
	}

	Buffer::Buffer(size_t nSize) {
		Buffer();
		Create(nSize);
	}

	Buffer::~Buffer() {
		Release();
	}

	bool Buffer::Create(size_t nSize) {
		Release();
		return Recreate(nSize);
	}

	bool Buffer::Recreate(size_t nNewSize) {
		if(m_nDataSize > nNewSize) {
			return false;
		}

		uint8_t* pNewBuf = (uint8_t*)realloc(m_pBuffer, nNewSize);
		if(!pNewBuf) {
			return false;
		}
		m_pBuffer = pNewBuf;
		m_nBufSize = nNewSize;

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

	bool Buffer::AppendData(size_t nSize) {
		return AppendData(NULL, nSize);
	}

	bool Buffer::AppendData(const uint8_t* pData, size_t nSize) {
		if(nSize > GetFreeSize()) {
			return false;
		}
		if(pData) {
			memcpy(GetBuffer(GetDataSize()), pData, nSize);
		}
		m_nDataSize += nSize;
		return true;
	}

	void Buffer::SkipData(size_t nSize) {
		size_t size = asl_min(m_nDataSize, nSize);
		m_nDataSize -= size;
		if(m_nDataSize > 0) {
			memmove(m_pBuffer, m_pBuffer + size, m_nDataSize);
		}
	}


	GrowthBuffer::GrowthBuffer() : Buffer() {
	}

	GrowthBuffer::GrowthBuffer(size_t nSize) : Buffer(nSize) {
	}

	GrowthBuffer::~GrowthBuffer() {
		Release();
	}

	bool GrowthBuffer::RequestFreeSize(size_t nSize) {
		if(GetFreeSize() >= nSize) {
			return true;
		}

		size_t nNewDataSize = GetDataSize() + nSize;
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
