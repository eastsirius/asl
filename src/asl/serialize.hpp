/**
 * @file serialize.hpp
 * @brief 序列化工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <cstdint>
#include <cassert>
#include <cstring>

#define ASL_ARCHIVABLE_BASE(n, v, b) \
	ASL_NAMESPACE::Archive::make_archivable(n, v, b)
#define ASL_ARCHIVABLE_VALUE(v) \
	ASL_ARCHIVABLE_BASE(#v, v, false)
#define ASL_ARCHIVABLE_NAMEVALUE(n, v) \
	ASL_ARCHIVABLE_BASE(n, v, false)
#define ASL_ARCHIVABLE_MYBE_VALUE(v) \
	ASL_ARCHIVABLE_BASE(#v, v, true)
#define ASL_ARCHIVABLE_MYBE_NAMEVALUE(n, v) \
	ASL_ARCHIVABLE_BASE(n, v, true)

namespace ASL_NAMESPACE {
	/**
	 * @class Bin
	 * @brief 二进制支持
	 */
	class Bin {
	public:
		/**
		 * @brief 反序列化1字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Read1Byte(const uint8_t* buf, void* value);

		/**
		 * @brief 序列化1字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Write1Byte(uint8_t* buf, const void* value);

		/**
		 * @brief 反序列化2字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Read2Byte(const uint8_t* buf, void* value);

		/**
		 * @brief 序列化2字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Write2Byte(uint8_t* buf, const void* value);

		/**
		 * @brief 反序列化3字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Read3Byte(const uint8_t* buf, void* value);

		/**
		 * @brief 序列化3字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Write3Byte(uint8_t* buf, const void* value);

		/**
		 * @brief 反序列化4字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Read4Byte(const uint8_t* buf, void* value);

		/**
		 * @brief 序列化4字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Write4Byte(uint8_t* buf, const void* value);

		/**
		 * @brief 反序列化8字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Read8Byte(const uint8_t* buf, void* value);

		/**
		 * @brief 序列化8字节类型
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		static void Write8Byte(uint8_t* buf, const void* value);
	};

	namespace helper {
		typedef void (*BinSerializerReader_t)(const uint8_t*, void*);
		typedef void (*BinSerializerWriter_t)(uint8_t*, const void*);
		BinSerializerReader_t g_BinSerializerReaders[];
		BinSerializerWriter_t g_BinSerializerWriters[];
	}

	/**
	 * @class BinSerializer
	 * @brief 二进制序列化器
	 */
	template <int N>
	struct BinSerializer {
	public:
		/**
		 * @brief 反序列化
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		inline static void DoRead(const uint8_t* buf, void* value) {
			helper::g_BinSerializerReaders[N](buf, value);
		}

		/**
		 * @brief 序列化
		 * @param buf 缓冲指针
		 * @param value 值指针
		 */
		inline static void DoWrite(uint8_t* buf, const void* value) {
			helper::g_BinSerializerWriters[N](buf, value);
		}
	};

	namespace helper {
		template <int L>
		struct StringSerializerHelper {
		};
		template <>
		struct StringSerializerHelper<1> {
			typedef uint8_t LengthType_t;
		};
		template <>
		struct StringSerializerHelper<2> {
			typedef uint16_t LengthType_t;
		};
		template <>
		struct StringSerializerHelper<3> {
			typedef uint32_t LengthType_t;
		};
		template <>
		struct StringSerializerHelper<4> {
			typedef uint32_t LengthType_t;
		};
		template <>
		struct StringSerializerHelper<8> {
			typedef uint64_t LengthType_t;
		};
	}

	/**
	 * @class StringSerializer
	 * @brief 字符串序列化器
	 */
	template <int N>
	struct StringSerializer {
	public:
		typedef typename helper::StringSerializerHelper<N>::LengthType_t LengthType_t;

		/**
		 * @brief 反序列化
		 * @param buf 缓冲指针
		 * @param size 缓冲长度
		 * @param value 值字符串
		 * @return 成功返回已处理缓冲长度，缓冲长度不足返回0
		 */
		inline static int DoRead(const uint8_t* buf, int size, std::string& value) {
			static_assert(N <= sizeof(std::string::size_type), "length type size too long");
			assert(buf != NULL && size > 0);

			if(size < N) {
				return 0;
			}

			LengthType_t len = 0;
			BinSerializer<N>::DoRead(buf, &len);

			if(size < (int)len + N) {
				return 0;
			}
			value.assign((const char*)buf + N, (int)len);

			return (int)len + N;
		}

		/**
		 * @brief 序列化
		 * @param buf 缓冲指针
		 * @param size 缓冲长度
		 * @param value 值字符串
		 * @return 成功返回已处理缓冲长度，缓冲长度不足返回0
		 */
		inline static int DoWrite(uint8_t* buf, int size, const std::string& value) {
			return DoWrite(buf, size, value.c_str(), value.length());
		}

		/**
		 * @brief 序列化
		 * @param buf 缓冲指针
		 * @param size 缓冲长度
		 * @param value 值字符串
		 * @param len 值字符串长度
		 * @return 成功返回已处理缓冲长度，缓冲长度不足返回0
		 */
		inline static int DoWrite(uint8_t* buf, int size, const char* value, std::string::size_type len) {
			static_assert(N <= sizeof(std::string::size_type), "length type size too long");
			assert(buf != NULL && size > 0);

			if(size < N + (int)len) {
				return 0;
			}

			LengthType_t len_len = LengthType_t(len);
			BinSerializer<N>::DoWrite(buf, &len_len);
			memcpy(buf + N, value, len);

			return len + N;
		}
	};

	/**
	 * @class ArchiveNull
	 * @brief 空类型
	 */
	class ArchiveNull {
	};

	/**
	 * @class ArchiveException
	 * @brief 存档异常
	 */
	class ArchiveException : public std::exception {
	public:
		ArchiveException(const char* what = NULL);
		virtual ~ArchiveException() throw();

	public:
		/**
		 * @brief 获取异常文本
		 * @return 返回异常文本
		 */
		virtual const char* what() const throw();

	protected:
		std::string m_strWhat;		///< 异常信息
	};

	/**
	 * @class Archive
	 * @brief 存档类
	 */
	class Archive : public NoCopyable {
	public:
		Archive() : m_bHasParam(false), m_nParam(0) {
		}

		/**
		 * @class archivable_t
		 * @brief 存档数据模版
		 */
		template <typename T>
		class archivable_t {
		public:
			archivable_t(const char* n, T& v, bool b):name(n),value(v),maybe(b){}

			const char* name;		///< 字段名
			T& value;				///< 字段值
			bool maybe;				///< 是否为可选项
		};

		/**
		 * @brief 生成存档数据
		 * @param n 字段名
		 * @param v 字段值
		 * @return 返回存档数据
		 */
		template <typename T>
		static archivable_t<T> make_archivable(const char* n, T& v, bool b) {
			return archivable_t<T>(n, v, b);
		}

		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const = 0;

		void SetGlobalParm(int parm) {
			m_bHasParam = true;
			m_nParam = parm;
		}

		int GetGlobalParm() const {
			return m_nParam;
		}

		bool HasParam() const {
			return m_bHasParam;
		}

	private:
		bool m_bHasParam;
		int m_nParam;
	};

	/**
	 * @class ArchiveSerializer
	 * @brief 自定义类型存档序列化工具
	 */
	template <class TA, typename T>
	class ArchiveSerializer {
	public:
		/**
		 * @brief 生成存档数据
		 * @param a 存档器
		 * @param v 数据字段
		 */
		void Serialize(TA& a, T& v) {
			v.Serialize(a);
		}
	};
}

/**
 * @class AslArchiveSerializer
 * @brief 自定义类型存档序列化工具
 */
template <class TA, typename T>
class AslArchiveSerializer : public ASL_NAMESPACE::ArchiveSerializer<TA, T> {
};
