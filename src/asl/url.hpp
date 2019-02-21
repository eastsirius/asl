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
	/**
	 * @brief URL
	 */
	class Url {
	public:
		Url();
		Url(const char* url);

	public:
		/**
		 * @brief 解析URL
		 * @param url 源URL
		 * @return 返回执行结果
		 */
		virtual bool Parse(const char* url);
		
		/**
		 * @brief 测试是否为空
		 * @return 返回测试结果
		 */
		bool IsEmpty() const {
			return m_strUrl.empty();
		}

		/**
		 * @brief 获取源URL
		 */
		const std::string& GetUrl() const {
			return m_strUrl;
		}
		/**
		 * @brief 获取协议名
		 */
		const std::string& GetProtocol() const {
			return m_strProtocol;
		}
		/**
		 * @brief 获取主机名
		 */
		const std::string& GetHost() const {
			return m_strHost;
		}
		/**
		 * @brief 获取用户名
		 */
		const std::string& GetUser() const {
			return m_strUser;
		}
		/**
		 * @brief 获取密码
		 */
		const std::string& GetPwd() const {
			return m_strPwd;
		}
		/**
		 * @brief 获取端口
		 */
		int GetPort() const {
			return m_nPort;
		}
		/**
		 * @brief 获取路径
		 */
		const std::string& GetPath() const {
			return m_strPath;
		}
		/**
		 * @brief 获取参数列表
		 */
		const std::map<std::string, std::string>& GetAttrMap() const {
			return m_mpAttrs;
		}

		/**
		 * @brief 判断参数是否存在
		 * @param key 参数名
		 * @return 返回参数是否存在
		 */
		bool HasAttr(const char* key) const;

		/**
		 * @brief 获取参数
		 * @param key 参数名
		 * @return 返回参数值
		 */
		const std::string& GetAttr(const char* key) const;

	public:
		/**
		 * @brief 获取协议端口
		 * @param protocol 协议名
		 * @return 成功返回协议端口，失败返回0
		 */
		static int GetProtocolPort(const char* protocol);

		/**
		 * @brief URL编码
		 * @param src 源字符串
		 * @param upper_case 转义字符是否为大写
		 * @return 返回编码后字符串
		 */
		static std::string UrlEncode(const char* src, bool upper_case = true);

		/**
		 * @brief URL解码
		 * @param src 源字符串
		 * @return 返回解码后字符串
		 */
		static std::string UrlDecode(const char* src);

		/**
		 * @brief 分割路径
		 * @param dirs 返回分级列表
		 * @param path 源路径
		 */
		static void SplitPath(std::vector<std::string>& dirs, const char* path);

	private:
		std::string m_strUrl;		///< 源URL

		std::string m_strProtocol;	///< 协议名
		std::string m_strHost;		///< 主机名
		std::string m_strUser;		///< 用户名
		std::string m_strPwd;		///< 密码
		int m_nPort;				///< 端口
		std::string m_strPath;		///< 路径
		std::map<std::string, std::string> m_mpAttrs;	///< 参数列表
	};
}
