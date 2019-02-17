/**
* @file jsonarchive.cpp
* @brief Json存档工具
* @author 程行通
*/

#include "jsonarchive.hpp"
#include <istream>
#include <ostream>

namespace ASL_NAMESPACE {
	JsonArchive::JsonArchive() : m_pJsonDoc(NULL) {
		m_ctxIoContext.pArchive = this;
		m_ctxIoContext.nArrayIndex = 0;
	}

	JsonArchive::~JsonArchive() {
		if(m_pJsonDoc != NULL) {
			delete m_pJsonDoc;
			m_pJsonDoc = NULL;
		}
	}

	JsonNode* JsonArchive::GetDoc() {
		return m_ctxIoContext.pNode;
	}

	JsonNode* JsonArchive::Death() {
		JsonNode* pNode = m_pJsonDoc;
		m_pJsonDoc = NULL;
		m_ctxIoContext.pNode = NULL;
		return pNode;
	}


	JsonInputArchive::JsonInputArchive(std::istream& isStream) {
		if(!isStream) {
			throw ArchiveException("input stream error");
		}

		isStream.seekg(0, std::istream::end);
		size_t sSize = (size_t)isStream.tellg();
		isStream.seekg(0, std::istream::beg);
		m_acDataBuffer.resize(sSize + 1);
		isStream.read(&m_acDataBuffer[0], sSize);
		m_acDataBuffer.back() = 0;

		const char* pErrPos = NULL;
		JsonNode* pRoot = JsonNode::Parse(&m_acDataBuffer[0], &pErrPos);
		if(pRoot == NULL) {
			char acErrBuf[32] = {0};
			if(pErrPos != NULL) {
				strncpy(acErrBuf, pErrPos, 31);
			}
			char acBuffer[128];
			sprintf(acBuffer, "parse json error, at pos: %s", acErrBuf);
			throw ArchiveException(acBuffer);
		}
		m_ctxIoContext.pNode = m_pJsonDoc = pRoot;
	}

	JsonInputArchive::JsonInputArchive(JsonNode* pNode) {
		if(pNode == NULL) {
			throw ArchiveException("input json node error");
		}

		m_ctxIoContext.pNode = m_pJsonDoc = pNode;
	}

	JsonInputArchive::~JsonInputArchive() {
	}


	JsonOutputArchive::JsonOutputArchive(std::ostream& osStream, bool bFormat) 
		: m_pStream(&osStream), m_bNeedFlush(false), m_bFormat(bFormat) {
		if(!*m_pStream) {
			throw ArchiveException("output stream error");
		}
		JsonNode* pRoot = new JsonObject();
		if(pRoot == NULL) {
			throw ArchiveException("create json node error");
		}
		m_ctxIoContext.pNode = m_pJsonDoc = pRoot;
	}

	JsonOutputArchive::JsonOutputArchive(JsonNode* pNode, bool bFormat)
		: m_pStream(NULL), m_bNeedFlush(false), m_bFormat(bFormat) {
		m_ctxIoContext.pNode = m_pJsonDoc = pNode;
	}

	JsonOutputArchive::~JsonOutputArchive() {
		Flush();
	}

	void JsonOutputArchive::Flush() {
		if(m_bNeedFlush && m_pStream != NULL) {
			if(!*m_pStream) {
				throw ArchiveException("error when write data");
			}

			assert(m_pJsonDoc != NULL);
			if(m_bFormat) {
				m_pJsonDoc->Print(*m_pStream);
			} else {
				m_pJsonDoc->PrintUnformatted(*m_pStream);
			}
			m_pStream->flush();

			m_bNeedFlush = false;
		}
	}
}
