/**
* @file binarchive.cpp
* @brief 二进制存档工具
* @author 程行通
* @copyright Copyright 2015 by Cheng Xingtong
*/

#include "binarchive.hpp"
#include <istream>
#include <ostream>

#define ASL_BINOUTPUTARCHINE_BUFSIZE 256 * 1024
#define ASL_BINOUTPUTARCHINE_GATESIZE 64 * 1024

namespace asl {
	BinArchive::BinArchive() {
		m_ctxIoContext.pArchive = this;
	}


	BinInputArchive::BinInputArchive(std::istream& isStream) : m_isStream(isStream) {
		if(!m_isStream) {
			throw ArchiveException("input stream error");
		}

		m_isStream.seekg(0, std::istream::end);
		size_t sSize = (size_t)m_isStream.tellg();
		m_isStream.seekg(0, std::istream::beg);
		m_acDataBuffer.resize(sSize);
		m_isStream.read(&m_acDataBuffer[0], sSize);

		m_ctxIoContext.pData = (uint8_t*)&m_acDataBuffer[0];
		m_ctxIoContext.nSize = (int)sSize;
	}

	BinInputArchive::~BinInputArchive() {
	}


	BinOutputArchive::BinOutputArchive(std::ostream& osStream) : m_osStream(osStream) {
		if(!m_osStream) {
			throw ArchiveException("output stream error");
		}

		m_acDataBuffer.resize(ASL_BINOUTPUTARCHINE_BUFSIZE);
		m_ctxIoContext.pData = (uint8_t*)&m_acDataBuffer[0];
		m_ctxIoContext.nSize = ASL_BINOUTPUTARCHINE_BUFSIZE;
	}

	BinOutputArchive::~BinOutputArchive() {
		Flush();
	}

	void BinOutputArchive::Flush() {
		assert(m_ctxIoContext.nSize >= 0 && m_ctxIoContext.nSize <=ASL_BINOUTPUTARCHINE_BUFSIZE);
		if(m_ctxIoContext.nSize < ASL_BINOUTPUTARCHINE_BUFSIZE) {
			if(!m_osStream) {
				throw ArchiveException("error when write data");
			}
			m_osStream.write(&m_acDataBuffer[0], ASL_BINOUTPUTARCHINE_BUFSIZE - m_ctxIoContext.nSize);
			m_ctxIoContext.pData = (uint8_t*)&m_acDataBuffer[0];
			m_ctxIoContext.nSize = ASL_BINOUTPUTARCHINE_BUFSIZE;
		}
	}

	void BinOutputArchive::TestAndFlush() {
		if(m_ctxIoContext.nSize < ASL_BINOUTPUTARCHINE_GATESIZE) {
			Flush();
		}
	}
}
