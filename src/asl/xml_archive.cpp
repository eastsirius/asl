/**
* @file xml_archive.cpp
* @brief XML存档工具
* @author 程行通
*/

#include "xml_archive.hpp"
#include <istream>
#include <ostream>
#include "rapidxml/rapidxml_print.hpp"

namespace ASL_NAMESPACE {
	XmlArchive::XmlArchive() {
		m_ctxIoContext.pArchive = this;
		m_ctxIoContext.pDoc = &m_xdDoc;
		m_ctxIoContext.pNode = &m_xdDoc;
	}


	XmlInputArchive::XmlInputArchive(std::istream& isStream) : m_isStream(isStream) {
		if(!m_isStream) {
			throw ArchiveException("input stream error");
		}

		m_isStream.seekg(0, std::istream::end);
		size_t sSize = (size_t)m_isStream.tellg();
		m_isStream.seekg(0, std::istream::beg);
		m_acDataBuffer.resize(sSize + 1);
		m_isStream.read(&m_acDataBuffer[0], sSize);
		m_acDataBuffer.back() = 0;
		try {
			m_xdDoc.parse<0>(&m_acDataBuffer[0]);
		} catch(std::exception& e) {
			throw ArchiveException(e.what());
		}
	}

	XmlInputArchive::~XmlInputArchive() {
	}


	XmlOutputArchive::XmlOutputArchive(std::ostream& osStream) : m_osStream(osStream), m_bNeedFlush(false) {
		if(!m_osStream) {
			throw ArchiveException("output stream error");
		}
	}

	XmlOutputArchive::~XmlOutputArchive() {
		Flush();
	}

	void XmlOutputArchive::Flush() {
		if(m_bNeedFlush) {
			if(!m_osStream) {
				throw ArchiveException("error when write data");
			}

			m_osStream << m_xdDoc;
			m_osStream.flush();

			m_bNeedFlush = false;
		}
	}
}
