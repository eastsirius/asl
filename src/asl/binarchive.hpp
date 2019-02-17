/**
* @file binarchive.hpp
* @brief 二进制存档工具
* @author 程行通
*/

#pragma once

#include "asldef.hpp"
#include "serialize.hpp"
#include <vector>
#include <map>
#include <list>
#include <cstring>
#include <cassert>

namespace asl {
	class BinInputArchive;
	class BinOutputArchive;
	namespace helper {
		/**
		 * @brief 二进制存档IO上下文
		 */
		struct BinArchiveIoContext_t {
			Archive* pArchive;						///< 存档器指针
			int nSize;								///< 数据长度
			uint8_t* pData;							///< 当前读写位置指针
		};

		/**
		 * @brief 二进制存档辅助器
		 */
		template <typename T>
		struct BinArchiveHelper {
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(BinArchiveIoContext_t& ctx, const Archive::archivable_t<T>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);
				
				AslArchiveSerializer<BinOutputArchive, T> serializer;
				serializer.Serialize(*(BinOutputArchive*)ctx.pArchive, v.value);
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(BinArchiveIoContext_t& ctx, const Archive::archivable_t<T>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);
				
				AslArchiveSerializer<BinInputArchive, T> serializer;
				serializer.Serialize(*(BinInputArchive*)ctx.pArchive, v.value);
			}
		};
		
#define ASL_BINARCHIVEHELPER(type) \
		template <> \
		struct BinArchiveHelper<type> { \
		public: \
			static void Serial(BinArchiveIoContext_t& ctx, const Archive::archivable_t<type>& v) { \
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0); \
				if(ctx.nSize < (int)sizeof(type)) { \
					throw ArchiveException("error when serial data"); \
				} \
				BinSerializer<sizeof(type)>::DoWrite(ctx.pData, &v.value); \
				ctx.pData += sizeof(type); \
				ctx.nSize -= sizeof(type); \
			} \
			static void Parse(BinArchiveIoContext_t& ctx, const Archive::archivable_t<type>& v) { \
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0); \
				if(ctx.nSize < (int)sizeof(type)) { \
					throw ArchiveException("error when parse data"); \
				} \
				BinSerializer<sizeof(type)>::DoRead(ctx.pData, &v.value); \
				ctx.pData += sizeof(type); \
				ctx.nSize -= (int)sizeof(type); \
			} \
		}; \

		ASL_BINARCHIVEHELPER(char)
		ASL_BINARCHIVEHELPER(signed char)
		ASL_BINARCHIVEHELPER(unsigned char)
		ASL_BINARCHIVEHELPER(short)
		ASL_BINARCHIVEHELPER(unsigned short)
		ASL_BINARCHIVEHELPER(int)
		ASL_BINARCHIVEHELPER(unsigned int)
		ASL_BINARCHIVEHELPER(long)
		ASL_BINARCHIVEHELPER(unsigned long)
		ASL_BINARCHIVEHELPER(long long)
		ASL_BINARCHIVEHELPER(unsigned long long)
		ASL_BINARCHIVEHELPER(float)
		ASL_BINARCHIVEHELPER(double)
		//ASL_BINARCHIVEHELPER(long double)
		//ASL_BINARCHIVEHELPER(bool)
		//ASL_BINARCHIVEHELPER(std::string)

#undef ASL_BINARCHIVEHELPER

		/**
		 * @brief 二进制存档辅助器
		 */
		template <>
		struct BinArchiveHelper<bool> {
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(BinArchiveIoContext_t& ctx, const Archive::archivable_t<bool>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);
				if(ctx.nSize < 1) {
					throw ArchiveException("error when serial data");
				}
				uint8_t ucValue = v.value ? 1 : 0;
				BinSerializer<1>::DoWrite(ctx.pData, &ucValue);
				ctx.pData += 1;
				ctx.nSize -= 1;
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(BinArchiveIoContext_t& ctx, const Archive::archivable_t<bool>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);
				if(ctx.nSize < 1) {
					throw ArchiveException("error when parse data");
				}
				uint8_t ucValue;
				BinSerializer<1>::DoRead(ctx.pData, &ucValue);
				v.value = ucValue != 0;
				ctx.pData += 1;
				ctx.nSize -= 1;
			}
		};
		/**
		 * @brief 二进制存档辅助器
		 */
		template <>
		struct BinArchiveHelper<std::string> {
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(BinArchiveIoContext_t& ctx, const Archive::archivable_t<std::string>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);
				if(ctx.nSize < (int)v.value.length() + 4) {
					throw ArchiveException("error when serial data");
				}
				uint32_t dwValue = (uint32_t)v.value.length();
				BinSerializer<4>::DoWrite(ctx.pData, &dwValue);
				memcpy(ctx.pData + 4, v.value.c_str(), v.value.length());
				ctx.pData += v.value.length() + 4;
				ctx.nSize -= (int)v.value.length() + 4;
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(BinArchiveIoContext_t& ctx, const Archive::archivable_t<std::string>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);
				if(ctx.nSize < 4) {
					throw ArchiveException("error when parse data");
				}
				uint32_t dwValue = 0;
				BinSerializer<4>::DoRead(ctx.pData, &dwValue);
				if(ctx.nSize < (int)dwValue + 4) {
					throw ArchiveException("error when parse data");
				}
				v.value.assign((const char*)ctx.pData + 4, dwValue);
				ctx.pData += dwValue + 4;
				ctx.nSize -= dwValue + 4;
			}
		};
	}

	/**
	 * @class BinArchive
	 * @brief 二进制存档类
	 */
	class BinArchive : public Archive {
	protected:
		BinArchive();

	protected:
		std::vector<char> m_acDataBuffer;				///< 读写缓存
		helper::BinArchiveIoContext_t m_ctxIoContext;	///< IO上下文
	};

	/**
	 * @class BinInputArchive
	 * @brief 二进制输入存档类
	 * @warning 该类所有操作可能抛出ArchiveException异常
	 */
	class BinInputArchive : public BinArchive {
	public:
		BinInputArchive(std::istream& isStream);
		~BinInputArchive();

	public:
		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const {
			return true;
		}

		template <typename T>
		BinInputArchive& operator& (const archivable_t<T>& v) {
			helper::BinArchiveHelper<T>::Parse(m_ctxIoContext, v);
			return *this;
		}
		template <typename T>
		BinInputArchive& operator>> (const archivable_t<T>& v) {
			helper::BinArchiveHelper<T>::Parse(m_ctxIoContext, v);
			return *this;
		}

	private:
		std::istream& m_isStream;	///< 输入流
	};

	/**
	 * @class BinOutputArchive
	 * @brief 二进制输出存档类
	 * @warning 该类所有操作可能抛出ArchiveException异常
	 */
	class BinOutputArchive : public BinArchive {
	public:
		BinOutputArchive(std::ostream& osStream);
		~BinOutputArchive();

	public:
		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const {
			return false;
		}

		template <typename T>
		BinOutputArchive& operator& (const archivable_t<T>& v) {
			helper::BinArchiveHelper<T>::Serial(m_ctxIoContext, v);
			TestAndFlush();
			return *this;
		}
		template <typename T>
		BinOutputArchive& operator<< (const archivable_t<T>& v) {
			helper::BinArchiveHelper<T>::Serial(m_ctxIoContext, v);
			TestAndFlush();
			return *this;
		}

		/**
		 * @brief 刷新流
		 */
		void Flush();

		/**
		 * @brief 检测并刷新流
		 */
		void TestAndFlush();

	private:
		std::ostream& m_osStream;	///< 输出流
	};
	
	namespace helper {
		/**
		 * @brief 二进制存档辅助器
		 */
		template <typename M, typename N>
		struct BinArchiveHelper<std::pair<M, N> > {
			typedef std::pair<M, N> ValueType_t;
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(BinArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);

				BinOutputArchive& a(*(BinOutputArchive*)ctx.pArchive);
				a & ASL_ARCHIVABLE_NAMEVALUE("Key", v.value.first);
				a & ASL_ARCHIVABLE_NAMEVALUE("Value", v.value.second);
			}

			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(BinArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) {
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0);

				BinInputArchive& a(*(BinInputArchive*)ctx.pArchive);
				a & ASL_ARCHIVABLE_NAMEVALUE("Key", v.value.first);
				a & ASL_ARCHIVABLE_NAMEVALUE("Value", v.value.second);
			}
		};

#define ASL_XMLARCHIVEHELPER_STDTEMPLATE(type, itemType, paramList, addproc) \
		template <paramList> \
		struct BinArchiveHelper<type > { \
			typedef type ValueType_t; \
		public: \
			static void Serial(BinArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) { \
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0); \
				BinOutputArchive& a(*(BinOutputArchive*)ctx.pArchive); \
				uint32_t dwCount = (uint32_t)v.value.size(); \
				a & ASL_ARCHIVABLE_NAMEVALUE("Count", dwCount); \
				for(typename ValueType_t::iterator iter = v.value.begin(); \
					iter != v.value.end(); ++iter) { \
					itemType item = *iter; \
					a & ASL_ARCHIVABLE_NAMEVALUE("Item", item); \
				} \
			} \
			static void Parse(BinArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) { \
				assert(ctx.pArchive != NULL && ctx.pData != NULL && ctx.nSize >= 0); \
				v.value.clear(); \
				BinInputArchive& a(*(BinInputArchive*)ctx.pArchive); \
				uint32_t dwCount = 0; \
				a & ASL_ARCHIVABLE_NAMEVALUE("Count", dwCount); \
				itemType item; \
				for(uint32_t i = 0; i < dwCount; ++i) { \
					a & ASL_ARCHIVABLE_NAMEVALUE("Item", item); \
					v.value.addproc(item); \
				} \
			} \
		}; \

		ASL_XMLARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::vector<N>), ASL_ONEPARAM(N), ASL_ONEPARAM(typename N), push_back)
		ASL_XMLARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::list<N>), ASL_ONEPARAM(N), ASL_ONEPARAM(typename N), push_back)
		ASL_XMLARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::map<M,N>), ASL_ONEPARAM(std::pair<M,N>), ASL_ONEPARAM(typename M,typename N), insert)
#undef ASL_XMLARCHIVEHELPER_STDTEMPLATE
	}
}
