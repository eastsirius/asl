/**
 * @file url.hpp
 * @brief URL支持
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <map>
#include <vector>
#include <cstring>

namespace ASL_NAMESPACE {
	class Url {
	public:
		Url();
		Url(const char* szUrl);

	public:
		virtual bool Parse(const char* szUrl);

		bool IsEmpty() const {
			return m_strUrl.empty();
		}

		const std::string& GetUrl() const {
			return m_strUrl;
		}
		const std::string& GetProtocol() const {
			return m_strProtocol;
		}
		const std::string& GetHost() const {
			return m_strHost;
		}
		int GetPort() const {
			return m_nPort;
		}
		const std::string& GetPath() const {
			return m_strPath;
		}
		const std::map<std::string, std::string>& GetAttrMap() const {
			return m_mpAttrs;
		}

		bool HasAttr(const char* strKey) const;
		const std::string& GetAttr(const char* strKey) const;

	public:
		static int GetProtocolPort(const char* szProtocol);
		static std::string UrlEncode(const char* szSrc);
		static std::string UrlDecode(const char* szSrc);
		static void SplitPath(std::vector<std::string>& lstDirs, const char* szPath);

	private:
		std::string m_strUrl;

		std::string m_strProtocol;
		std::string m_strHost;
		int m_nPort;
		std::string m_strPath;
		std::map<std::string, std::string> m_mpAttrs;
	};
}
