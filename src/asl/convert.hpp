/**
 * @file convert.hpp
 * @brief 转换工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <string>
#include <cstdint>
#include <cassert>

namespace ASL_NAMESPACE {
	namespace helper {
		/**
		 * @class ConvertHelper
		 * @brief 类型转换辅助类
		 */
		class ConvertHelper {
		public:
			/**
			 * @brief 获取字符串长度
			 * @param str 源字符串
			 * @return 返回字符串长度
			 */
			static int GetWStringLength(const wchar_t* str);

			/**
			 * @brief Unicode转换为本地字符集
			 * @param str 源字符串
			 * @return 返回转换后字符串
			 */
			static std::string UnicodeToAnsi(const wchar_t* str);

			/**
			 * @brief 本地字符集转换为Unicode
			 * @param str 源字符串
			 * @return 返回转换后字符串
			 */
			static std::wstring AnsiToUnicode(const char* str);

			/**
			 * @brief Unicode转换为Utf8
			 * @param str 源字符串
			 * @return 返回转换后字符串
			 */
			static std::string UnicodeToUtf8(const wchar_t* str);

			/**
			 * @brief Utf8转换为Unicode
			 * @param str 源字符串
			 * @return 返回转换后字符串
			 */
			static std::wstring Utf8ToUnicode(const char* str);
		};

#if 1
#define ASL_DOTYPECONVERT_SIMPLEVALUE2(type, type2, fmt1, fmt2, buf) \
		template <> \
		struct DoTypeConvert<std::string, type> { \
		public: \
			inline static void Do(std::string& dest, const type& src) { \
				char acBuffer[buf]; \
				sprintf(acBuffer, fmt1, (type2)src); \
				dest = acBuffer; \
			} \
		}; \
		template <> \
		struct DoTypeConvert<type, std::string> { \
		public: \
			inline static void Do(type& dest, const std::string& src) { \
				type2 nValue = 0; \
				sscanf(src.c_str(), fmt2, &nValue); \
				dest = (type)nValue; \
			} \
		}; \

#define ASL_DOTYPECONVERT_SIMPLEVALUE(type, type2, fmt, buf) \
	ASL_DOTYPECONVERT_SIMPLEVALUE2(type, type2, fmt, fmt, buf)

		template <typename T1, typename T2>
		struct DoTypeConvert {
		public:
			inline static void Do(T1& dest, const T2& src) {
				assert(false);
			}
		};
		template <>
		struct DoTypeConvert<std::string, std::string> {
		public:
			inline static void Do(std::string& dest, const std::string& src) {
				dest = src;
			}
		};

		ASL_DOTYPECONVERT_SIMPLEVALUE(char, int, "%d", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(signed char, int, "%d", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(unsigned char, unsigned int, "%u", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(short, int, "%d", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(unsigned short, unsigned int, "%u", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(int, int, "%d", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(unsigned int, unsigned int, "%u", 32)
		ASL_DOTYPECONVERT_SIMPLEVALUE(long, ASLPRINT_INT64_TYPE, ASLPRINT_INT64, 64)
		ASL_DOTYPECONVERT_SIMPLEVALUE(unsigned long, ASLPRINT_UINT64_TYPE, ASLPRINT_UINT64, 64)
		ASL_DOTYPECONVERT_SIMPLEVALUE(long long, ASLPRINT_INT64_TYPE, ASLPRINT_INT64, 64)
		ASL_DOTYPECONVERT_SIMPLEVALUE(unsigned long long, ASLPRINT_UINT64_TYPE, ASLPRINT_UINT64, 64)
		ASL_DOTYPECONVERT_SIMPLEVALUE2(float, double, "%.16lg", "%lg", 64)
		ASL_DOTYPECONVERT_SIMPLEVALUE2(double, double, "%.16lg", "%lg", 64)
		ASL_DOTYPECONVERT_SIMPLEVALUE2(long double, double, "%.16lg", "%lg", 64)

		template <>
		struct DoTypeConvert<std::string, bool> {
		public:
			inline static void Do(std::string& dest, const bool& src) {
				dest = src ? "true" : "false";
			}
		};
		template <>
		struct DoTypeConvert<bool, std::string> {
		public:
			inline static void Do(bool& dest, const std::string& src) {
				if(src == "true") {
					dest = true;
				} else if(src == "false") {
					dest = false;
				} else {
					assert(false);
					dest = false;
				}
			}
		};

#undef ASL_DOTYPECONVERT_SIMPLEVALUE
#undef ASL_DOTYPECONVERT_SIMPLEVALUE2
#endif
	}

	/**
	 * @enum StringCodecID
	 * @brief 字符串编码类型
	 */
	enum StringCodecID {
		ASCID_ANSI,			///<本地字符集
		ASCID_UTF8,			///<UTF8

		ASCID_Unknown
	};

	template <int nCodecID>
	class to_widebyte_cast {
	public:
		to_widebyte_cast(const char* str) {
			assert(false);
		}

	public:
		operator const wchar_t* () const {
			return m_strValue.c_str();
		}
		operator wchar_t* () const {
			return (wchar_t*)m_strValue.c_str();
		}
		operator const wchar_t* () {
			return m_strValue.c_str();
		}
		operator wchar_t* () {
			return (wchar_t*)m_strValue.c_str();
		}

	private:
		std::wstring m_strValue;
	};
	template <>
	class to_widebyte_cast<ASCID_ANSI> {
	public:
		to_widebyte_cast(const char* str) {
			m_strValue = helper::ConvertHelper::AnsiToUnicode(str);
		}

	public:
		operator const wchar_t* () const {
			return m_strValue.c_str();
		}
		operator wchar_t* () const {
			return (wchar_t*)m_strValue.c_str();
		}
		operator const wchar_t* () {
			return m_strValue.c_str();
		}
		operator wchar_t* () {
			return (wchar_t*)m_strValue.c_str();
		}

	private:
		std::wstring m_strValue;
	};
	template <>
	class to_widebyte_cast<ASCID_UTF8> {
	public:
		to_widebyte_cast(const char* str) {
			m_strValue = helper::ConvertHelper::Utf8ToUnicode(str);
		}

	public:
		operator const wchar_t* () const {
			return m_strValue.c_str();
		}
		operator wchar_t* () const {
			return (wchar_t*)m_strValue.c_str();
		}
		operator const wchar_t* () {
			return m_strValue.c_str();
		}
		operator wchar_t* () {
			return (wchar_t*)m_strValue.c_str();
		}

	private:
		std::wstring m_strValue;
	};

	template <int nCodecID>
	class from_widebyte_cast {
	public:
		from_widebyte_cast(const wchar_t* str) {
			assert(false);
		}

	public:
		operator const char* () const {
			return m_strValue.c_str();
		}
		operator char* () const {
			return (char*)m_strValue.c_str();
		}
		operator const char* () {
			return m_strValue.c_str();
		}
		operator char* () {
			return (char*)m_strValue.c_str();
		}

	private:
		std::string m_strValue;
	};
	template <>
	class from_widebyte_cast<ASCID_ANSI> {
	public:
		from_widebyte_cast(const wchar_t* str) {
			m_strValue = helper::ConvertHelper::UnicodeToAnsi(str);
		}

	public:
		operator const char* () const {
			return m_strValue.c_str();
		}
		operator char* () const {
			return (char*)m_strValue.c_str();
		}
		operator const char* () {
			return m_strValue.c_str();
		}
		operator char* () {
			return (char*)m_strValue.c_str();
		}

	private:
		std::string m_strValue;
	};
	template <>
	class from_widebyte_cast<ASCID_UTF8> {
	public:
		from_widebyte_cast(const wchar_t* str) {
			m_strValue = helper::ConvertHelper::UnicodeToUtf8(str);
		}

	public:
		operator const char* () const {
			return m_strValue.c_str();
		}
		operator char* () const {
			return (char*)m_strValue.c_str();
		}
		operator const char* () {
			return m_strValue.c_str();
		}
		operator char* () {
			return (char*)m_strValue.c_str();
		}

	private:
		std::string m_strValue;
	};

	template <typename valueType>
	class to_string_cast {
	public:
		to_string_cast(valueType src) {
			helper::DoTypeConvert<std::string, valueType>::Do(m_strValue, src);
		}

	public:
		operator const char* () const {
			return m_strValue.c_str();
		}
		operator char* () const {
			return (char*)m_strValue.c_str();
		}
		operator const char* () {
			return m_strValue.c_str();
		}
		operator char* () {
			return (char*)m_strValue.c_str();
		}

	private:
		std::string m_strValue;
	};

	template <typename valueType>
	class from_string_cast {
	public:
		from_string_cast(std::string src) {
			helper::DoTypeConvert<valueType, std::string>::Do(m_Value, src);
		}

	public:
		operator valueType() const {
			return m_Value;
		}
		operator valueType() {
			return m_Value;
		}

	private:
		valueType m_Value;
	};


	/**
	 * @brief 二进制数据转换为十六进制字符串
	 * @param dest 返回十六进制字符串
	 * @param data 数据指针
	 * @param size 数据长度
	 * @param bUpCase 字母大小写
	 */
	void DataToHexString(std::string& dest, const uint8_t* data, int size, bool up_case);

	/**
	 * @brief 二进制数据转换为十六进制字符串
	 * @param data 数据指针
	 * @param size 数据长度
	 * @param up_case 字母大小写
	 * @return 返回十六进制字符串
	 */
	std::string DataToHexString(const uint8_t* data, int size, bool up_case);
}
