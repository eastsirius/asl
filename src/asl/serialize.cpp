/**
 * @file serialize.cpp
 * @brief 序列化工具
 * @author 程行通
 */

#include "serialize.hpp"

namespace ASL_NAMESPACE {
	void Bin::Read1Byte(const uint8_t* buf, void* value) {
		*(uint8_t*)value = *buf;
	}

	void Bin::Write1Byte(uint8_t* buf, const void* value) {
		*buf = *(uint8_t*)value;
	}

	void Bin::Read2Byte(const uint8_t* buf, void* value) {
		uint16_t v = 0;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		memcpy(value, &v, 2);
	}

	void Bin::Write2Byte(uint8_t* buf, const void* value) {
		uint16_t v = 0;
		memcpy(&v, value, 2);
		*buf = uint8_t((v >>  8) & 0xFF);
		++buf;
		*buf = uint8_t((v      ) & 0xFF);
	}

	void Bin::Read3Byte(const uint8_t* buf, void* value) {
		uint32_t v = 0;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		memcpy(value, &v, 4);
	}

	void Bin::Write3Byte(uint8_t* buf, const void* value) {
		uint32_t v = 0;
		memcpy(&v, value, 4);
		*buf = uint8_t((v >> 16) & 0xFF);
		++buf;
		*buf = uint8_t((v >>  8) & 0xFF);
		++buf;
		*buf = uint8_t((v      ) & 0xFF);
	}

	void Bin::Read4Byte(const uint8_t* buf, void* value) {
		uint32_t v = 0;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		memcpy(value, &v, 4);
	}

	void Bin::Write4Byte(uint8_t* buf, const void* value) {
		uint32_t v = 0;
		memcpy(&v, value, 4);
		*buf = uint8_t((v >> 24) & 0xFF);
		++buf;
		*buf = uint8_t((v >> 16) & 0xFF);
		++buf;
		*buf = uint8_t((v >>  8) & 0xFF);
		++buf;
		*buf = uint8_t((v      ) & 0xFF);
	}

	void Bin::Read8Byte(const uint8_t* buf, void* value) {
		uint64_t v = 0;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		v <<= 8;
		++buf;
		v |= *buf;
		memcpy(value, &v, 8);
	}

	void Bin::Write8Byte(uint8_t* buf, const void* value) {
		uint64_t v = 0;
		memcpy(&v, value, 8);
		*buf = uint8_t((v >> 56) & 0xFF);
		++buf;
		*buf = uint8_t((v >> 48) & 0xFF);
		++buf;
		*buf = uint8_t((v >> 40) & 0xFF);
		++buf;
		*buf = uint8_t((v >> 32) & 0xFF);
		++buf;
		*buf = uint8_t((v >> 24) & 0xFF);
		++buf;
		*buf = uint8_t((v >> 16) & 0xFF);
		++buf;
		*buf = uint8_t((v >>  8) & 0xFF);
		++buf;
		*buf = uint8_t((v      ) & 0xFF);
	}


	namespace helper {
		BinSerializerReader_t g_BinSerializerReaders[] = {
			NULL,
			&Bin::Read1Byte,
			&Bin::Read2Byte,
			&Bin::Read3Byte,
			&Bin::Read4Byte,
			NULL, NULL, NULL,
			&Bin::Read8Byte,
		};
		BinSerializerWriter_t g_BinSerializerWriters[] = {
			NULL,
			&Bin::Write1Byte,
			&Bin::Write2Byte,
			&Bin::Write3Byte,
			&Bin::Write4Byte,
			NULL, NULL, NULL,
			&Bin::Write8Byte,
		};
	}


	ArchiveException::ArchiveException(const char* what) {
		m_strWhat = what == NULL ? "archive error" : what;
	}

	ArchiveException::~ArchiveException() throw() {
	}

	const char* ArchiveException::what() const throw() {
		return m_strWhat.c_str();
	}
}
