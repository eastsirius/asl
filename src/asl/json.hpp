/**
* @file json.hpp
* @brief JSON协议支持
* @author 程行通
*/

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <string>
#include <stdint.h>
#include <ostream>
#include <cassert>
#include <map>
#include <vector>
#include <exception>

namespace asl {
	/**
     * @brief JSON节点类型
	 */
	enum JsonNodeType {
		JNT_String,		///< 字符串类型
		JNT_Number,		///< 数值类型
		JNT_Object,		///< 类类型
		JNT_Array,		///< 数组类型
		JNT_True,		///< 布尔类型true
		JNT_False,		///< 布尔类型false
		JNT_Null,		///< 空类型

		JNT_Unknown		///< 未知类型
	};

	/**
     * @brief JSON节点
	 */
	class JsonNode : public NoCopyable {
	public:
		JsonNode();
		virtual ~JsonNode();

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const = 0;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 */
		virtual void Print(std::ostream& os);

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os) = 0;

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const = 0;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt) = 0;

		/**
		 * @brief 解析节点
		 * @param szSrc 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* szSrc, const char** err_pos) = 0;

		/**
		 * @brief 解析节点
		 * @param szSrc 源字符串
		 * @param err_pos 返回错误位置
		 * @return 成功返回节点，失败返回NULL
		 */
		static JsonNode* Parse(const char* szSrc, const char** err_pos = NULL);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param parsed 已解析长度
		 * @param err_pos 返回错误位置
		 * @return 成功返回节点，失败返回NULL
		 */
		static JsonNode* Parse2(const char* src, int& parsed, const char** err_pos = NULL);

	protected:
		/**
		 * @brief 打印制表符
		 * @param os 输出流
		 * @param count 缩进计数
		 */
		static inline void _PrintTable(std::ostream& os, int count) {
			for(int i = 0; i < count; ++i) {
				os << "\t";
			}
		}
	};

	/**
     * @brief JSON字符串节点
	 */
	class JsonString : public JsonNode {
	public:
		JsonString();
		JsonString(const char* value);
		virtual ~JsonString();

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const {
			return JNT_String;
		}

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os);

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* src, const char** err_pos);

		operator const char* () const {
			return m_strValue.c_str();
		}

	private:
		/**
		 * @brief 解析单个16进制字符
		 * @param src 源字符
		 * @return 成功返回数值，失败返回负数
		 */
		static int _ParseHex(char src);

		/**
		 * @brief 解析16进制字符
		 * @param value 源字符串
		 * @return 返回数值
		 */
		static int _ParseHex4(const char* value);

		/**
		 * @brief 写入UTF8字符串
		 * @param dest 目标缓存
		 * @param src 源字符
		 */
		static void _WriteUtf8(std::vector<char>& dest, uint32_t src);

	protected:
		std::string m_strValue;		///< 节点值
	};

	/**
     * @brief JSON数值节点
	 */
	class JsonNumber : public JsonNode {
	public:
		JsonNumber();
#define ASL_JSON_NUMBER_INITPROC_INT(type) \
		explicit JsonNumber(type value) \
			: m_bFloat(false), m_nValue((int64_t)value) \
		{ } \

#define ASL_JSON_NUMBER_INITPROC_FLOAT(type) \
		explicit JsonNumber(type value) \
			: m_bFloat(true), m_dbValue((double)value) \
		{ } \

		ASL_JSON_NUMBER_INITPROC_INT(int8_t)
		ASL_JSON_NUMBER_INITPROC_INT(int16_t)
		ASL_JSON_NUMBER_INITPROC_INT(int32_t)
		ASL_JSON_NUMBER_INITPROC_INT(int64_t)
		ASL_JSON_NUMBER_INITPROC_INT(uint8_t)
		ASL_JSON_NUMBER_INITPROC_INT(uint16_t)
		ASL_JSON_NUMBER_INITPROC_INT(uint32_t)
		ASL_JSON_NUMBER_INITPROC_INT(uint64_t)
		ASL_JSON_NUMBER_INITPROC_FLOAT(float)
		ASL_JSON_NUMBER_INITPROC_FLOAT(double)

#undef ASL_JSON_NUMBER_INITPROC_FLOAT
#undef ASL_JSON_NUMBER_INITPROC_INT
		virtual ~JsonNumber();

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const {
			return JNT_Number;
		}

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os);

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* src, const char** err_pos);

		/**
		 * @brief 获取是否为浮点数
		 * @return 返回是否为浮点数
		 */
		bool FloatOrInt() const {
			return m_bFloat;
		}

		operator int64_t() const {
			return int64_t(m_bFloat ? (int64_t)m_dbValue : m_nValue);
		}

		operator int() const {
			return int(m_bFloat ? (int64_t)m_dbValue : m_nValue);
		}

		operator double() const {
			return double(m_bFloat ? m_dbValue : (double)m_nValue);
		}

		operator float() const {
			return float(m_bFloat ? m_dbValue : (double)m_nValue);
		}

	protected:
		bool m_bFloat;			///< 是否为浮点型
		union {
			int64_t m_nValue;	///< 整形节点值
			double m_dbValue;	///< 浮点型节点值
		};
	};

	/**
     * @brief JSON布尔值节点
	 */
	class JsonBool : public JsonNode {
	public:
		JsonBool();
		JsonBool(bool value);
		virtual ~JsonBool();

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const {
			return m_bValue ? JNT_True : JNT_False;
		}

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os);

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* src, const char** err_pos);

		operator bool() const {
			return m_bValue;
		}

	protected:
		bool m_bValue;		///< 节点值
	};

	/**
     * @brief JSON空节点
	 */
	class JsonNull : public JsonNode {
	public:
		JsonNull();
		virtual ~JsonNull();

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const {
			return JNT_Null;
		}

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os);

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* src, const char** err_pos);
	};

	/**
     * @class JsonObject
     * @brief JSON类节点
	 */
	class JsonObject : public JsonNode {
	public:
		JsonObject();
		virtual ~JsonObject();

		typedef std::map<std::string, JsonNode*> ItemMap_t;	///< 项键值表类型

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const {
			return JNT_Object;
		}

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os);

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* src, const char** err_pos);

		/**
		 * @brief 获取项计数
		 * @return 返回项计数
		 */
		int GetItemCount() const;

		/**
		 * @brief 获取项
		 * @param name 项名
		 * @return 成功返回项指针,失败返回NULL
		 */
		JsonNode* GetItem(const char* name) const;

		/**
		 * @brief 设置项
		 * @param name 项名
		 * @param node 项节点指针
		 */
		void SetItem(const char* name, JsonNode* node);

		/**
		 * @brief 移除项
		 * @param name 项名
		 */
		void RemoveItem(const char* name);

		/**
		 * @brief 分离项
		 * @param name 项名
		 * @return 成功返回项指针,无该项返回NULL
		 */
		JsonNode* DeathItem(const char* name);

	protected:
		ItemMap_t m_mpItemMap;		///< 项键值表
	};

	/**
     * @class JsonArray
     * @brief JSON数组节点
	 */
	class JsonArray : public JsonNode {
	public:
		JsonArray();
		virtual ~JsonArray();

		typedef std::vector<JsonNode*> ItemList_t;	///< 项列表类型

	public:
		/**
		 * @brief 获取节点类型
		 * @return 返回节点类型
		 */
		virtual JsonNodeType GetType() const {
			return JNT_Array;
		}

		/**
		 * @brief 非格式化的打印节点
		 * @param os 输出流
		 */
		virtual void PrintUnformatted(std::ostream& os);

		/**
		 * @brief 克隆节点
		 * @return 返回克隆的节点
		 */
		virtual JsonNode* Clone() const;

		/**
		 * @brief 打印节点
		 * @param os 输出流
		 * @param nt 缩进计数
		 */
		virtual void _Print(std::ostream& os, int nt);

		/**
		 * @brief 解析节点
		 * @param src 源字符串
		 * @param err_pos 返回错误位置
		 * @return 返回已解析长度
		 */
		virtual int _Parse(const char* src, const char** err_pos);

		/**
		 * @brief 获取项计数
		 * @return 返回项计数
		 */
		int GetItemCount() const;

		/**
		 * @brief 获取项
		 * @param index 项索引
		 * @return 成功返回项,失败返回NULL
		 */
		JsonNode* GetItem(int index) const;

		/**
		 * @brief 添加项
		 * @param node 项节点指针
		 */
		void AddItem(JsonNode* node);

		/**
		 * @brief 设置项
		 * @param index 项索引
		 * @param node 项节点指针
		 * @return 返回执行结果
		 */
		bool SetItem(int index, JsonNode* node);

		/**
		 * @brief 插入项
		 * @param index 位置索引
		 * @param node 项节点指针
		 * @return 返回执行结果
		 */
		bool InsertItem(int index, JsonNode* node);

		/**
		 * @brief 删除项
		 * @param index 位置索引
		 */
		void DeleteItem(int index);

	protected:
		ItemList_t m_lstItemList;			  ///< 项列表
	};
}
