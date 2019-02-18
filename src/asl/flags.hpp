/**
 * @file flags.hpp
 * @brief 程序选项解析
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace ASL_NAMESPACE {
	/**
     * @brief 选项基类
     */
	class FlagBase {
	public:
		FlagBase(const char* szKey, const char* szShortKey, const char* szUsage);
		virtual ~FlagBase();

	public:
		/**
		 * @brief 解析选项
		 * @param szValue 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* szValue) = 0;

		/**
		 * @brief 打印用法
		 * @param nKeyLen 键值最大长度
		 */
		void PrintUsage(int nKeyLen) const;

		/**
		 * @brief 获取键值
		 */
		const std::string& GetKey() const {
			return m_strKey;
		}

		/**
		 * @brief 获取键值缩写
		 */
		const std::string& GetShortKey() const {
			return m_strShortKey;
		}

		/**
		 * @brief 获取用法描述
		 */
		const std::string& GetUsage() const {
			return m_strUsage;
		}

	private:
		/**
		 * @brief 打印健值
		 * @param nKeyLen 键值最大长度
		 * @return 返回执行结果
		 */
		std::string _PrintKey(int nKeyLen) const;

	private:
		const std::string m_strKey;
		const std::string m_strShortKey;
		const std::string m_strUsage;
	};

	/**
     * @brief 字符串选项
     */
	class StringFlag : public FlagBase {
	public:
		StringFlag(std::string& strValue, const char* szKey, const char* szShortKey,
				const char* szUsage, std::string strDefaultValue);
		virtual ~StringFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param szValue 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* szValue);

	private:
		std::string& m_strValue; ///< 选项值
	};

	/**
     * @brief 数值选项
     */
	class IntFlag : public FlagBase {
	public:
		IntFlag(int& nValue, const char* szKey, const char* szShortKey,
				const char* szUsage, int nDefaultValue);
		virtual ~IntFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param szValue 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* szValue);

	private:
		int& m_nValue; ///< 选项值
	};

	/**
     * @brief 布尔选项
     */
	class BoolFlag : public FlagBase {
	public:
		BoolFlag(bool& bValue, const char* szKey, const char* szShortKey,
				 const char* szUsage, bool bDefaultValue);
		virtual ~BoolFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param szValue 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* szValue);

	private:
		bool& m_bValue; ///< 选项值
	};

	/**
     * @brief 浮点数值选项
     */
	class FloatFlag : public FlagBase {
	public:
		FloatFlag(float& fValue, const char* szKey, const char* szShortKey,
				  const char* szUsage, float fDefaultValue);
		virtual ~FloatFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param szValue 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* szValue);

	private:
		float& m_fValue; ///< 选项值
	};

	/**
     * @brief 程序选项
     */
	class Flags {
	public:
		Flags();

		typedef std::shared_ptr<FlagBase> FlagPtr_t;

	public:
		/**
		 * @brief 解析程序选项
		 * @param argc 参数数
		 * @param args 参数列表
		 * @return 返回执行结果
		 */
		bool Parse(int argc, const char** args);

		/**
		 * @brief 打印用法
		 */
		void Usage();

		/**
		 * @brief 添加字符串选项
		 * @param strValue 绑定变量
		 * @param szKey 健值
		 * @param szShortKey 健值缩写
		 * @param szUsage 用法描述
		 * @param strDefaultValue 默认值
		 */
		void StringValue(std::string& strValue, const char* szKey, const char* szShortKey,
				const char* szUsage, std::string strDefaultValue = "");

		/**
		 * @brief 添加数值选项
		 * @param strValue 绑定变量
		 * @param szKey 健值
		 * @param szShortKey 健值缩写
		 * @param szUsage 用法描述
		 * @param strDefaultValue 默认值
		 */
		void IntValue(int& nValue, const char* szKey, const char* szShortKey,
				const char* szUsage, int nDefaultValue = 0);

		/**
		 * @brief 添加布尔选项
		 * @param strValue 绑定变量
		 * @param szKey 健值
		 * @param szShortKey 健值缩写
		 * @param szUsage 用法描述
		 * @param strDefaultValue 默认值
		 */
		void BoolValue(bool& bValue, const char* szKey, const char* szShortKey,
				const char* szUsage, bool bDefaultValue = false);

		/**
		 * @brief 添加浮点数值选项
		 * @param strValue 绑定变量
		 * @param szKey 健值
		 * @param szShortKey 健值缩写
		 * @param szUsage 用法描述
		 * @param strDefaultValue 默认值
		 */
		void FloatValue(float& fValue, const char* szKey, const char* szShortKey,
				const char* szUsage, float fDefaultValue = 0.0f);

	private:
		/**
		 * @brief 添加选项
		 * @param szKey 健值
		 * @param szShortKey 健值缩写
		 * @param pValue 选项实例
		 */
		void _AddValue(const char* szKey, const char* szShortKey, FlagBase* pValue);

	private:
		std::map<std::string, FlagPtr_t> m_mpValues;
		std::map<std::string, FlagPtr_t> m_mpKeyValueMap;
		std::map<std::string, FlagPtr_t> m_mpShortKeyValueMap;
	};
}
