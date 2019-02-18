/**
 * @file url.cpp
 * @brief URL支持
 * @author 程行通
 */

#include "url.hpp"
#include "utils.hpp"
#include <cstdint>
#include <cassert>

namespace ASL_NAMESPACE {
	Url::Url() : m_nPort(0) {
	}

	Url::Url(const char *url) : m_nPort(0) {
		Parse(url);
	}

	bool Url::Parse(const char* url) {
		*this = Url();

		std::string u(url);
		std::string::size_type protocol_end_pos = u.find("://");
		if(protocol_end_pos == std::string::npos) {
			return false;
		}
		std::string protocol(u.begin(), u.begin() + protocol_end_pos);
		std::string temp(u.begin() + protocol_end_pos + 3, u.end());

		std::string::size_type addr_end_pos = temp.find_first_of("/");
		std::string addr = temp;
		if(addr_end_pos != std::string::npos) {
			addr = std::string(temp.begin(), temp.begin() + addr_end_pos);
			temp = std::string(temp.begin() + addr_end_pos + 1, temp.end());
		} else {
			temp = "";
		}

		std::string::size_type host_end_pos = addr.find_first_of(":");
		std::string host;
		int port;
		if(host_end_pos == std::string::npos) {
			host = addr;
			port = GetProtocolPort(protocol.c_str());
		} else {
			host = std::string(addr.begin(), addr.begin() + host_end_pos);
			std::string port_str(addr.begin() + host_end_pos + 1, addr.end());
			port = atoi(port_str.c_str());
			if(port <= 0) {
				return false;
			}
		}

		std::string::size_type path_end_pos = temp.find_first_of("?");
		std::string path("/");
		if(path_end_pos == std::string::npos) {
			path += temp;
			temp = "";
		} else {
			path += std::string(temp.begin(), temp.begin() + path_end_pos);
			temp = std::string(temp.begin() + path_end_pos + 1, temp.end());
		}

		m_strUrl = url;
		m_strProtocol = protocol;
		m_strHost = host;
		m_nPort = port;
		m_strPath = path;

		return true;
	}

	bool Url::HasAttr(const char* key) const {
		return m_mpAttrs.find(key) != m_mpAttrs.end();
	}

	const std::string& Url::GetAttr(const char* key) const {
		auto iter = m_mpAttrs.find(key);
		if(iter != m_mpAttrs.end()) {
			return iter->second;
		} else {
			static std::string null_string;
			return null_string;
		}
	}

	int Url::GetProtocolPort(const char* protocol) {
		// TODO
		if(strcasecmp(protocol, "rtsp") == 0) {
			return 443;
		} else if(strcasecmp(protocol, "rtspu") == 0) {
			return 443;
		} else if(strcasecmp(protocol, "http") == 0) {
			return 80;
		} else {
			return 0;
		}
	}

	std::string Url::UrlEncode(const char* src, bool upper_case) {
		assert(src != NULL);
		if(src == NULL) {
			return "";
		}

		char baseChar = upper_case ? 'A' : 'a';
		std::vector<uint8_t> lstDest;
		uint8_t c;
		uint8_t* pSrc = (uint8_t*)src;
		while(*pSrc) {
			c = *pSrc;
			if(isalpha(c) || isdigit(c) || c == '-' || c == '.' || c == '~') {
				lstDest.push_back(c);
			} else if(c == ' ') {
				lstDest.push_back('+');
			} else {
				lstDest.push_back('%');
				lstDest.push_back(uint8_t((c >= 0xA0) ? ((c >> 4) - 10 + baseChar) : ((c >> 4) + '0')));
				lstDest.push_back(uint8_t(((c & 0xF) >= 0xA) ? ((c & 0xF) - 10 + baseChar) : ((c & 0xF) + '0')));
			}
			++pSrc;
		}
		lstDest.push_back(0);

		return (char*)&lstDest[0];
	}

	std::string Url::UrlDecode(const char* src) {
		assert(src != NULL);
		if(src == NULL) {
			return "";
		}

		uint8_t* pSrc = (uint8_t*)src;
		std::vector<uint8_t> lstDest;
		uint8_t c;
		while(*pSrc) {
			if(*pSrc == '%') {
				c = 0;

				//高位
				if(pSrc[1] >= 'A' && pSrc[1] <= 'F') {
					c += (pSrc[1] - 'A' + 10) * 0x10;
				} else if(pSrc[1] >= 'a' && pSrc[1] <= 'f') {
					c += (pSrc[1] - 'a' + 10) * 0x10;
				} else {
					c += (pSrc[1] - '0') * 0x10;
				}

				//低位
				if(pSrc[2] >= 'A' && pSrc[2] <= 'F') {
					c += (pSrc[2] - 'A' + 10);
				} else if(pSrc[2] >= 'a' && pSrc[2] <= 'f') {
					c += (pSrc[2] - 'a' + 10);
				} else {
					c += (pSrc[2] - '0');
				}

				lstDest.push_back(c);
				pSrc += 3;
			} else if(*pSrc == '+') {
				lstDest.push_back(' ');
				++pSrc;
			} else {
				lstDest.push_back(*pSrc);
				++pSrc;
			}
		}

		lstDest.push_back(0);

		return (char*)&lstDest[0];
	}

	void Url::SplitPath(std::vector<std::string>& dirs, const char* path) {
		std::string p(path);
		dirs.clear();

		std::string::size_type pos;
		std::string lhs, rhs;
		do {
			pos = p.find_first_of("/");
			if(pos == std::string::npos) {
				lhs = path;
				rhs = "";
			} else {
				lhs = std::string(p.begin(), p.begin() + pos);
				rhs = std::string(p.begin() + pos + 1, p.end());
			}
			if(lhs != "") {
				dirs.push_back(lhs);
			}
			p = rhs;
		} while(pos != std::string::npos);
	}
}
