/**
* @file json.cpp
* @brief JSON协议支持
* @author 程行通
*/

#include "json.hpp"
#include "utils.hpp"
#include "convert.hpp"

namespace ASL_NAMESPACE {
	JsonNode::JsonNode() {
	}

	JsonNode::~JsonNode() {
	}

	void JsonNode::Print(std::ostream& os) {
		_Print(os, 0);
	}

	JsonNode* JsonNode::Parse(const char* src, const char** err_pos) {
		int parsed = 0;
		return Parse2(src, parsed, err_pos);
	}
	
	JsonNode* JsonNode::Parse2(const char* src, int& parsed, const char** err_pos) {
		assert(src != NULL);
		parsed = 0;
		if(err_pos) {
			*err_pos = NULL;
		}
		const char* pSrc = src;
		bool bIsNodes = false;
		while(*pSrc) {
			if(bIsNodes) {
				if(*pSrc == '\n') {
					bIsNodes = false;
				}
				++pSrc;
			} else {
				while(*pSrc && !isgraph(*pSrc)) {
					++pSrc;
				}
				if(pSrc[0] == '/' && pSrc[1] == '/') {
					bIsNodes = true;
				} else {
					break;
				}
			}
		}

		JsonNode* pNode = NULL;
		switch(*pSrc) {
		case '\"':
			pNode = new JsonString();
			break;

		case '{':
			pNode = new JsonObject();
			break;

		case '[':
			pNode = new JsonArray();
			break;

		case 't':
		case 'f':
			pNode = new JsonBool();
			break;

		case 'n':
			pNode = new JsonNull();
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '+':
		case '-':
			pNode = new JsonNumber();
			break;

		default:
			break;
		}

		if(pNode != NULL) {
			int nRet = pNode->_Parse(pSrc, err_pos);
			if(nRet < 0) {
				delete pNode;
				return NULL;
			}
			parsed = (int)(pSrc - src + nRet);
		} else {
			if(err_pos) {
				*err_pos = pSrc;
			}
		}

		return pNode;
	}


	JsonString::JsonString() {
	}

	JsonString::JsonString(const char* value) {
		if(value) {
			m_strValue = value;
		}
	}
	
	JsonString::~JsonString() {
	}

	void JsonString::PrintUnformatted(std::ostream& os) {
		_Print(os, 0);
	}

	JsonNode* JsonString::Clone() const {
		return new JsonString(m_strValue.c_str());
	}

	void JsonString::_Print(std::ostream& os, int nt) {
		os << "\"";

		char acBuffer[8] = {0};
		for(const char* pSrc = m_strValue.c_str(); *pSrc != 0; ++pSrc) {
			if((unsigned char)*pSrc > 31 && *pSrc != '\"' && *pSrc != '\\') {
				os << *pSrc;
			} else {
				os << '\\';
				switch(*pSrc) {
				case '\\':
					os << '\\';
					break;
				case '\"':
					os << '\"';
					break;
				case '\b':
					os << 'b';
					break;
				case '\f':
					os << 'f';
					break;
				case '\n':
					os << 'n';
					break;
				case '\r':
					os << 'r';
					break;
				case '\t':
					os << 't';
					break;
				default:
					sprintf(acBuffer, "u%04x", (int)(uint8_t)*pSrc);
					os << acBuffer;
					break;
				}
			}
		}

		os << "\"";
	}

	int JsonString::_Parse(const char* src, const char** err_pos) {
		assert(src != NULL);
		assert(*src == '"');

		const char* szBegin = src;
		if(*(src++) != '"') {
			if(err_pos) {
				*err_pos = src - 1;
			}
			return -1;
		}

		std::vector<char> lstString;
		while(true) {
			if(*src == 0) {
				if(err_pos)
				{
					*err_pos = src;
				}
				return -1;
			} else if(*src == '"') {
				++src;
				break;
			} else if(*src == '\\') {
				++src;
				switch(*src) {
				case '"':
					lstString.push_back('\"');
					++src;
					break;
				case '\\':
					lstString.push_back('\\');
					++src;
					break;
				case '/':
					lstString.push_back('/');
					++src;
					break;
				case 'b':
					lstString.push_back('\b');
					++src;
					break;
				case 'f':
					lstString.push_back('\f');
					++src;
					break;
				case 'n':
					lstString.push_back('\n');
					++src;
					break;
				case 'r':
					lstString.push_back('\r');
					++src;
					break;
				case 't':
					lstString.push_back('\t');
					++src;
					break;

				case 'u':
					if(isxdigit(src[1]) && isxdigit(src[2]) &&
						isxdigit(src[3]) && isxdigit(src[4])) {
						bool bDoubleValue = false;
						uint32_t dwChar = _ParseHex4(src + 1);
						if((dwChar >= 0xDC00 && dwChar <= 0xDFFF) || dwChar == 0) {
							if(err_pos) {
								*err_pos = src;
							}
							return -1;
						} else if(dwChar >= 0xD800 && dwChar <= 0xDBFF) {
							bDoubleValue = true;
							if(src[5] == '\\' && src[6] == 'u' && isxdigit(src[7]) 
								&& isxdigit(src[8]) && isxdigit(src[9]) && isxdigit(src[10])) {
								uint32_t dwTemp = _ParseHex4(src + 7);
								if(dwTemp >= 0xDC00 || dwTemp <= 0xDFFF) {
									dwChar = 0x10000 + (((dwChar & 0x3FF) << 10) | (dwTemp & 0x3FF));
								} else {
									if(err_pos) {
										*err_pos = src;
									}
									return -1;
								}
							} else {
								if(err_pos) {
									*err_pos = src;
								}
								return -1;
							}
						}
						src += bDoubleValue ? 11 : 5;
					} else {
						if(err_pos) {
							*err_pos = src;
						}
						return -1;
					}
					break;

				default:
					if(err_pos) {
						*err_pos = src;
					}
					return -1;
					break;
				}
			} else {
				lstString.push_back(*(src++));
			}
		}
		lstString.push_back(0);
		m_strValue = &lstString[0];

		return (int)(src - szBegin);
	}

	int JsonString::_ParseHex(char src) {
		if(src >= '0' && src <= '9') {
			return src - '0';
		} else if(src >= 'A' && src <= 'F') {
			return 10 + src - 'A';
		} else if(src >= 'a' && src <= 'f') {
			return 10 + src - 'a';
		} else {
			return -1;
		}
	}

	int JsonString::_ParseHex4(const char* value) {
		int nValue = 0;
		for(int i = 0; i < 4; ++i) {
			int nRet = _ParseHex(*value);
			assert(nRet >= 0);
			nValue <<= 4;
			nValue |= nRet;
			++value;
		}

		return nValue;
	}

	void JsonString::_WriteUtf8(std::vector<char>& dest, uint32_t src) {
		if(src < 0x00080) {
			dest.push_back((char)(src & 0xFF));
		} else if(src < 0x00800) {
			dest.push_back((char)(0xC0 + ((src >> 6) & 0x1F)));
			dest.push_back((char)(0x80 + (src & 0x3F)));
		} else if(src < 0x10000) {
			dest.push_back((char)(0xE0 + ((src >> 12) & 0x0F)));
			dest.push_back((char)(0x80 + ((src >> 6) & 0x3F)));
			dest.push_back((char)(0x80 + (src & 0x3F)));
		} else {
			dest.push_back((char)(0xF0 + ((src >> 18) & 0x07)));
			dest.push_back((char)(0x80 + ((src >> 12) & 0x3F)));
			dest.push_back((char)(0x80 + ((src >> 6) & 0x3F)));
			dest.push_back((char)(0x80 + (src & 0x3F)));
		}
	}


	JsonNumber::JsonNumber() : m_bFloat(false), m_nValue(0) {
	}
	
	JsonNumber::~JsonNumber() {
	}

	void JsonNumber::PrintUnformatted(std::ostream& os) {
		_Print(os, 0);
	}

	JsonNode* JsonNumber::Clone() const {
		auto ret = new JsonNumber();
		ret->m_bFloat = m_bFloat;
		if(m_bFloat) {
			ret->m_dbValue = m_dbValue;
		} else {
			ret->m_nValue = m_nValue;
		}

		return ret;
	}

	void JsonNumber::_Print(std::ostream& os, int nt) {
		if(m_bFloat) {
			os << (const char*)to_string_cast<double>(m_dbValue);
		} else {
			os <<(const char*) to_string_cast<int64_t>(m_nValue);
		}
	}

	int JsonNumber::_Parse(const char* src, const char** err_pos) {
		assert(src != NULL);
		assert(isgraph(*src));

		const char* pSrc = src;
		std::vector<char> lstString;
		bool bLoop = true;
		m_bFloat = false;
		while(bLoop) {
			switch(*pSrc) {
			case '.':
			case 'e':
			case 'E':
				m_bFloat = true;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '+':
			case '-':
				lstString.push_back(*pSrc);
				break;

			default:
				bLoop = false;
				break;
			}
			++pSrc;
		}

		if(lstString.empty() || lstString[0] == '.' || lstString[0] == 'e' || lstString[0] == 'E') {
			if(err_pos) {
				*err_pos = src;
				return -1;
			}
		}
		lstString.push_back(0);
		if(m_bFloat) {
			m_dbValue = from_string_cast<double>(&lstString[0]);
		} else {
			m_nValue = from_string_cast<int64_t>(&lstString[0]);
		}

		return (int)lstString.size() - 1;
	}


	JsonBool::JsonBool() : m_bValue(false) {
	}

	JsonBool::JsonBool(bool bValue) : m_bValue(bValue) {
	}

	JsonBool::~JsonBool() {
	}

	void JsonBool::PrintUnformatted(std::ostream& os) {
		_Print(os, 0);
	}

	JsonNode* JsonBool::Clone() const {
		return new JsonBool(m_bValue);
	}

	void JsonBool::_Print(std::ostream& os, int nt) {
		os << (m_bValue ? "true" : "false");
	}

	int JsonBool::_Parse(const char* src, const char** err_pos) {
		assert(src != NULL);
		assert(isgraph(*src));

		if(memcmp(src, "true", 4) == 0) {
			m_bValue = true;
			return 4;
		}
		if(memcmp(src, "false", 5) == 0) {
			m_bValue = false;
			return 5;
		} else {
			if(err_pos) {
				*err_pos = src;
			}
			return -1;
		}
	}

	
	JsonNull::JsonNull() {
	}
	
	JsonNull::~JsonNull() {
	}
	
	void JsonNull::PrintUnformatted(std::ostream& os) {
		_Print(os, 0);
	}

	JsonNode* JsonNull::Clone() const {
		return new JsonNull();
	}

	void JsonNull::_Print(std::ostream& os, int nt) {
		os << "null";
	}

	int JsonNull::_Parse(const char* src, const char** err_pos) {
		assert(src != NULL);
		assert(isgraph(*src));

		if(memcmp(src, "null", 4) == 0) {
			return 4;
		} else {
			if(err_pos) {
				*err_pos = src;
			}
			return -1;
		}
	}

	
	JsonObject::JsonObject() {
	}
	
	JsonObject::~JsonObject() {
		for(ItemMap_t::const_iterator iter = m_mpItemMap.begin();
			iter != m_mpItemMap.end(); ++iter) {
			delete iter->second;
		}
		m_mpItemMap.clear();
	}
	
	void JsonObject::PrintUnformatted(std::ostream& os) {
		os << "{";
		bool bFirst = true;
		for(ItemMap_t::const_iterator iter = m_mpItemMap.begin();
			iter != m_mpItemMap.end(); ++iter) {
			if(bFirst) {
				bFirst = false;
			} else {
				os << ",";
			}
			os << "\"" << iter->first.c_str() << "\":";
			iter->second->PrintUnformatted(os);
		}
		os << "}";
	}

	JsonNode* JsonObject::Clone() const {
		JsonObject* pObject = new JsonObject();
		for(ItemMap_t::const_iterator iter = m_mpItemMap.begin();
			iter != m_mpItemMap.end(); ++iter) {
			pObject->m_mpItemMap.insert(ItemMap_t::value_type(iter->first, iter->second->Clone()));
		}

		return pObject;
	}

	void JsonObject::_Print(std::ostream& os, int nt) {
		os << "{\n";
		bool bFirst = true;
		for(ItemMap_t::const_iterator iter = m_mpItemMap.begin();
			iter != m_mpItemMap.end(); ++iter) {
			if(bFirst) {
				bFirst = false;
			} else {
				os << ",\n";
			}
			_PrintTable(os, nt + 1);
			os << "\"" << iter->first.c_str() << "\": ";
			iter->second->_Print(os, nt + 1);
		}
		os << "\n";
		_PrintTable(os, nt);
		os << "}";
	}

	int JsonObject::_Parse(const char* src, const char** err_pos) {
		assert(src != NULL);
		assert(*src == '{');

		const char* pSrc = src;
		if(*(pSrc++) != '{') {
			if(err_pos) {
				*err_pos = pSrc - 1;
			}
			return -1;
		}

		while(true) {
			while(*pSrc && !isgraph(*pSrc)) {
				++pSrc;
			}

			switch(*pSrc) {
			case '}':
				return (int)(pSrc + 1 - src);
				break;

			case ',':
				++pSrc;
				break;

			case 0:
				if(err_pos) {
					*err_pos = pSrc;
				}
				return -1;
				break;

			default:
				{
					int nRet = 0;
					JsonNode* pKeyNode = JsonNode::Parse2(pSrc, nRet, err_pos);
					if(pKeyNode == NULL || pKeyNode->GetType() != JNT_String) {
						if(pKeyNode != NULL) {
							delete pKeyNode;
						}
						return -1;
					}
					std::string strKey = (const char*)*(JsonString*)pKeyNode;
					delete pKeyNode;
					pSrc += nRet;

					while(*pSrc && !isgraph(*pSrc)) {
						++pSrc;
					}
					if(*pSrc != ':') {
						if(err_pos) {
							*err_pos = pSrc;
						}
						return -1;
					}
					++pSrc;

					JsonNode* pNode = JsonNode::Parse2(pSrc, nRet, err_pos);
					if(pNode == NULL) {
						return -1;
					}
					SetItem(strKey.c_str(), pNode);
					pSrc += nRet;
				}
				break;
			}
		}

		return -1;
	}

	int JsonObject::GetItemCount() const {
		return (int)m_mpItemMap.size();
	}

	JsonNode* JsonObject::GetItem(const char* name) const {
		assert(name != NULL);
		ItemMap_t::const_iterator iter = m_mpItemMap.find(name);
		if(iter == m_mpItemMap.end()) {
			return NULL;
		} else {
			return iter->second;
		}
	}

	void JsonObject::SetItem(const char* name, JsonNode* node) {
		assert(name != NULL);
		assert(node != NULL);
		if(node == NULL) {
			return;
		}

		ItemMap_t::iterator iter = m_mpItemMap.find(name);
		if(iter == m_mpItemMap.end()) {
			m_mpItemMap.insert(ItemMap_t::value_type(name, node));
		} else {
			delete iter->second;
			iter->second = node;
		}
	}

	void JsonObject::RemoveItem(const char* name) {
		assert(name != NULL);
		m_mpItemMap.erase(name);
	}

	JsonNode* JsonObject::DeathItem(const char* name) {
		assert(name != NULL);
		ItemMap_t::const_iterator iter = m_mpItemMap.find(name);
		if(iter == m_mpItemMap.end()) {
			return NULL;
		} else {
			JsonNode* pNode = iter->second;
			m_mpItemMap.erase(iter->first);
			return pNode;
		}
	}


	JsonArray::JsonArray() {
	}
	
	JsonArray::~JsonArray() {
		for(size_t i = 0; i < m_lstItemList.size(); ++i) {
			delete m_lstItemList[i];
		}
		m_lstItemList.clear();
	}
	
	void JsonArray::PrintUnformatted(std::ostream& os) {
		os << "[";
		bool bFirst = true;
		for(size_t i = 0; i < m_lstItemList.size(); ++i) {
			if(bFirst) {
				bFirst = false;
			} else {
				os << ",";
			}
			m_lstItemList[i]->PrintUnformatted(os);
		}
		os << "]";
	}

	JsonNode* JsonArray::Clone() const {
		JsonArray* pArray = new JsonArray();
		for(size_t i = 0; i < m_lstItemList.size(); ++i) {
			pArray->m_lstItemList.push_back(m_lstItemList[i]->Clone());
		}

		return pArray;
	}

	void JsonArray::_Print(std::ostream& os, int nt) {
		os << "[\n";
		bool bFirst = true;
		for(size_t i = 0; i < m_lstItemList.size(); ++i) {
			if(bFirst) {
				bFirst = false;
			} else {
				os << ",\n";
			}
			_PrintTable(os, nt + 1);
			m_lstItemList[i]->_Print(os, nt + 1);
		}
		os << "\n";
		_PrintTable(os, nt);
		os << "]";
	}

	int JsonArray::_Parse(const char* src, const char** err_pos) {
		assert(src != NULL);
		assert(*src == '[');

		const char* pSrc = src;
		if(*(pSrc++) != '[') {
			if(err_pos) {
				*err_pos = pSrc - 1;
			}
			return -1;
		}

		while(true) {
			while(*pSrc && !isgraph(*pSrc)) {
				++pSrc;
			}

			switch(*pSrc) {
			case ']':
				return (int)(pSrc + 1 - src);
				break;

			case ',':
				++pSrc;
				break;

			case 0:
				if(err_pos) {
					*err_pos = pSrc;
				}
				return -1;
				break;

			default:
				{
					int nRet = 0;
					JsonNode* pNode = JsonNode::Parse2(pSrc, nRet, err_pos);
					if(pNode == NULL) {
						return -1;
					}
					m_lstItemList.push_back(pNode);
					pSrc += nRet;
				}
				break;
			}
		}

		return -1;
	}

	int JsonArray::GetItemCount() const {
		return (int)m_lstItemList.size();
	}

	JsonNode* JsonArray::GetItem(int index) const {
		if(index >= 0 && index < (int)m_lstItemList.size()) {
			return m_lstItemList[index];
		} else {
			return NULL;
		}
	}

	void JsonArray::AddItem(JsonNode* node) {
		assert(node != NULL);
		if(node != NULL) {
			m_lstItemList.push_back(node);
		}
	}

	bool JsonArray::SetItem(int index, JsonNode* node) {
		assert(node != NULL);
		if(index >= 0 && index < (int)m_lstItemList.size() && node != NULL) {
			delete m_lstItemList[index];
			m_lstItemList[index] = node;
			return true;
		} else {
			return false;
		}
	}

	bool JsonArray::InsertItem(int index, JsonNode* node) {
		assert(node != NULL);
		if(index >= 0 && index <= (int)m_lstItemList.size() && node != NULL) {
			m_lstItemList.insert(m_lstItemList.begin() + index, node);
			return true;
		} else {
			return false;
		}
	}

	void JsonArray::DeleteItem(int index) {
		if(index >= 0 && index < (int)m_lstItemList.size()) {
			delete m_lstItemList[index];
			m_lstItemList.erase(m_lstItemList.begin() + index);
		}
	}
}
