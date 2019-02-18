/**
 * @file url.cpp
 * @brief URL支持
 * @author 程行通
 */

#include "url.hpp"
#include "utils.hpp"

namespace ASL_NAMESPACE {
	Url::Url() : m_nPort(0) {
	}

	Url::Url(const char *szUrl) : Url() {
		Parse(szUrl);
	}

	bool Url::Parse(const char *szUrl) {
		*this = Url();

		std::string url(szUrl);
		std::string::size_type protocol_end_pos = url.find("://");
		if(protocol_end_pos == std::string::npos) {
			return false;
		}
		std::string protocol(url.begin(), url.begin() + protocol_end_pos);
		std::string temp(url.begin() + protocol_end_pos + 3, url.end());

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

	bool Url::HasAttr(const char* strKey) const {
		return m_mpAttrs.find(strKey) != m_mpAttrs.end();
	}

	const std::string& Url::GetAttr(const char* strKey) const {
		auto iter = m_mpAttrs.find(strKey);
		if(iter != m_mpAttrs.end()) {
			return iter->second;
		} else {
			static std::string null_string;
			return null_string;
		}
	}

	int Url::GetProtocolPort(const char* szProtocol) {
		if(strcasecmp(szProtocol, "rtsp") == 0) {
			return 443;
		} else if(strcasecmp(szProtocol, "rtspu") == 0) {
			return 443;
		} else if(strcasecmp(szProtocol, "http") == 0) {
			return 80;
		} else {
			return 0;
		}
	}

	std::string Url::UrlEncode(const char* szSrc) {
		// TODO
		return szSrc;
	}

	std::string Url::UrlDecode(const char* szSrc) {
		// TODO
		return szSrc;
	}

	void Url::SplitPath(std::vector<std::string>& lstDirs, const char* szPath) {
		std::string path(szPath);
		lstDirs.clear();

		std::string::size_type pos;
		std::string lhs, rhs;
		do {
			pos = path.find_first_of("/");
			if(pos == std::string::npos) {
				lhs = path;
				rhs = "";
			} else {
				lhs = std::string(path.begin(), path.begin() + pos);
				rhs = std::string(path.begin() + pos + 1, path.end());
			}
			if(lhs != "") {
				lstDirs.push_back(lhs);
			}
			path = rhs;
		} while(pos != std::string::npos);
	}
}
