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
		FlagBase(const char* key, const char* short_key, const char* usage);
		virtual ~FlagBase();

	public:
		/**
		 * @brief 解析选项
		 * @param value 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* value) = 0;

		/**
		 * @brief 打印用法
		 * @param key_len 键值最大长度
		 */
		void PrintUsage(int key_len) const;

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
		 * @param key_len 键值最大长度
		 * @return 返回执行结果
		 */
		std::string _PrintKey(int key_len) const;

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
		StringFlag(std::string& value, const char* key, const char* short_key,
				const char* usage, std::string default_value);
		virtual ~StringFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param value 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* value);

	private:
		std::string& m_strValue; ///< 选项值
	};

	/**
     * @brief 数值选项
     */
	class IntFlag : public FlagBase {
	public:
		IntFlag(int& value, const char* key, const char* short_key,
				const char* usage, int default_value);
		virtual ~IntFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param value 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* value);

	private:
		int& m_nValue; ///< 选项值
	};

	/**
     * @brief 布尔选项
     */
	class BoolFlag : public FlagBase {
	public:
		BoolFlag(bool& value, const char* key, const char* short_key,
				 const char* usage, bool default_value);
		virtual ~BoolFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param value 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* value);

	private:
		bool& m_bValue; ///< 选项值
	};

	/**
     * @brief 浮点数值选项
     */
	class FloatFlag : public FlagBase {
	public:
		FloatFlag(float& value, const char* key, const char* short_key,
				  const char* usage, float default_value);
		virtual ~FloatFlag();

	public:
		/**
		 * @brief 解析选项
		 * @param value 选项值
		 * @return 返回执行结果
		 */
		virtual bool ParseFlag(const char* value);

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
		 * @param value 绑定变量
		 * @param key 健值
		 * @param short_key 健值缩写
		 * @param usage 用法描述
		 * @param default_value 默认值
		 */
		void StringValue(std::string& value, const char* key, const char* short_key,
				const char* usage, std::string default_value = "");

		/**
		 * @brief 添加数值选项
		 * @param value 绑定变量
		 * @param key 健值
		 * @param short_key 健值缩写
		 * @param usage 用法描述
		 * @param default_value 默认值
		 */
		void IntValue(int& value, const char* key, const char* short_key,
				const char* usage, int default_value = 0);

		/**
		 * @brief 添加布尔选项
		 * @param value 绑定变量
		 * @param key 健值
		 * @param short_key 健值缩写
		 * @param usage 用法描述
		 * @param default_value 默认值
		 */
		void BoolValue(bool& value, const char* key, const char* short_key,
				const char* usage, bool default_value = false);

		/**
		 * @brief 添加浮点数值选项
		 * @param value 绑定变量
		 * @param key 健值
		 * @param short_key 健值缩写
		 * @param usage 用法描述
		 * @param default_value 默认值
		 */
		void FloatValue(float& value, const char* key, const char* short_key,
				const char* usage, float default_value = 0.0f);

	private:
		/**
		 * @brief 添加选项
		 * @param key 健值
		 * @param short_key 健值缩写
		 * @param value 选项实例
		 */
		void _AddValue(const char* key, const char* short_key, FlagBase* value);

	private:
		std::map<std::string, FlagPtr_t> m_mpValues;
		std::map<std::string, FlagPtr_t> m_mpKeyValueMap;
		std::map<std::string, FlagPtr_t> m_mpShortKeyValueMap;
	};
}
