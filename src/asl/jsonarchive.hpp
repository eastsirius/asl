/**
* @file jsonarchive.hpp
* @brief Json存档工具
* @author 程行通
*/

#pragma once

#include "asldef.hpp"
#include "serialize.hpp"
#include "convert.hpp"
#include "json.hpp"
#include <vector>
#include <map>
#include <list>

namespace ASL_NAMESPACE {
	class JsonInputArchive;
	class JsonOutputArchive;
	namespace helper {
		/**
		 * @brief Json存档IO上下文
		 */
		struct JsonArchiveIoContext_t {
			Archive* pArchive;	///< 存档器指针
			JsonNode* pNode;		///< 当前X节点
			int nArrayIndex;	///< 数组索引
		};

		/**
		 * @brief Json存档辅助器
		 */
		template <typename T>
		struct JsonArchiveHelper {
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<T>& v) {
				assert(ctx.pArchive != NULL && ctx.pNode != NULL);
				assert(v.name != NULL);
				JsonNode* pNode = ctx.pNode;
				JsonNode* pChild = new JsonObject();
				if(pChild == NULL) {
					throw ArchiveException("error when alloc json node");
				}
				if(pNode->GetType() == JNT_Array) {
					((JsonArray*)pNode)->AddItem(pChild);
				} else if(pNode->GetType() == JNT_Object) {
					((JsonObject*)pNode)->SetItem(v.name, pChild);
				} else {
					assert(false);
					throw ArchiveException("error when add item");
				}

				AslArchiveSerializer<JsonOutputArchive, T> serializer;
				ctx.pNode = pChild;
				serializer.Serialize(*(JsonOutputArchive*)ctx.pArchive, v.value);
				ctx.pNode = pNode;
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<T>& v) {
				assert(ctx.pArchive != NULL && ctx.pNode != NULL);
				assert(v.name != NULL);
				JsonNode* pNode = ctx.pNode;
				JsonNode* pChild = NULL;
				if(pNode->GetType() == JNT_Array) {
					assert(ctx.nArrayIndex < ((JsonArray*)pNode)->GetItemCount());
					pChild = ((JsonArray*)pNode)->GetItem(ctx.nArrayIndex);
				} else if(pNode->GetType() == JNT_Object) {
					pChild = ((JsonObject*)pNode)->GetItem(v.name);
				} else {
					assert(false);
					throw ArchiveException("error when get item");
				}
				
				if(pChild != NULL && pChild->GetType() != JNT_Null) {
					ctx.pNode = pChild;
					AslArchiveSerializer<JsonInputArchive, T> serializer;
					serializer.Serialize(*(JsonInputArchive*)ctx.pArchive, v.value);
					ctx.pNode = pNode;
				} else if(!v.maybe) {
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str());
				}
			}
		};
		
		template <typename T>
		struct JsonNodeArchiveHelper {
			static JsonNode* Serial(const T& value);
			static void Parse(JsonNode* pNode, T& value);
		};
		template <>
		struct JsonNodeArchiveHelper<int64_t> {
			static JsonNode* Serial(const int64_t& value) {
				return new JsonNumber(value);
			}
			static void Parse(JsonNode* pNode, int64_t& value) {
				assert(pNode != NULL);
				if(pNode->GetType() == JNT_Number) {
					value = (int64_t)*(JsonNumber*)pNode;
				}
			}
		};
		template <>
		struct JsonNodeArchiveHelper<double> {
			static JsonNode* Serial(const double& value) {
				return new JsonNumber(value);
			}
			static void Parse(JsonNode* pNode, double& value) {
				assert(pNode != NULL);
				if(pNode->GetType() == JNT_Number)
				{
					value = (double)*(JsonNumber*)pNode;
				}
			}
		};
		template <>
		struct JsonNodeArchiveHelper<bool> {
			static JsonNode* Serial(const bool& value) {
				return new JsonBool(value);
			}
			static void Parse(JsonNode* pNode, bool& value) {
				assert(pNode != NULL);
				if(pNode->GetType() == JNT_True || pNode->GetType() == JNT_False) {
					value = *(JsonBool*)pNode;
				}
			}
		};
		template <>
		struct JsonNodeArchiveHelper<std::string> {
			static JsonNode* Serial(const std::string& value) {
				return new JsonString(value.c_str());
			}
			static void Parse(JsonNode* pNode, std::string& value) {
				assert(pNode != NULL);
				if(pNode->GetType() == JNT_String) {
					value = (const char*)*(JsonString*)pNode;
				}
			}
		};

#define ASL_JSONARCHIVEHELPER(oT, iT) \
		template <> \
		struct JsonArchiveHelper<oT> { \
		public: \
			static void Serial(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<oT>& v) { \
				assert(ctx.pArchive != NULL && ctx.pNode != NULL); \
				assert(v.name != NULL); \
				JsonNode* pNode = ctx.pNode; \
				iT value = (iT)v.value; \
				JsonNode* pChild = JsonNodeArchiveHelper<iT>::Serial(value); \
				if(pChild == NULL) { \
					throw ArchiveException("error when alloc json node"); \
				} \
				if(pNode->GetType() == JNT_Array) { \
					((JsonArray*)pNode)->AddItem(pChild); \
				} else if(pNode->GetType() == JNT_Object) { \
					((JsonObject*)pNode)->SetItem(v.name, pChild); \
				} else { \
					assert(false); \
					throw ArchiveException("error when add item"); \
				} \
			} \
			static void Parse(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<oT>& v) { \
				assert(ctx.pArchive != NULL && ctx.pNode != NULL); \
				assert(v.name != NULL); \
				JsonNode* pNode = ctx.pNode; \
				JsonNode* pChild = NULL; \
				if(pNode->GetType() == JNT_Array) { \
					assert(ctx.nArrayIndex < ((JsonArray*)pNode)->GetItemCount()); \
					pChild = ((JsonArray*)pNode)->GetItem(ctx.nArrayIndex); \
				} else if(pNode->GetType() == JNT_Object) { \
					pChild = ((JsonObject*)pNode)->GetItem(v.name); \
				} else { \
					assert(false); \
					throw ArchiveException("error when get item"); \
				} \
				if(pChild != NULL && pChild->GetType() != JNT_Null) { \
					iT value = (iT)v.value; \
					JsonNodeArchiveHelper<iT>::Parse(pChild, value); \
					v.value = (oT)value; \
				} else if(!v.maybe) { \
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str()); \
				} \
			} \
		}; \

		ASL_JSONARCHIVEHELPER(char, int64_t)
		ASL_JSONARCHIVEHELPER(signed char, int64_t)
		ASL_JSONARCHIVEHELPER(unsigned char, int64_t)
		ASL_JSONARCHIVEHELPER(short, int64_t)
		ASL_JSONARCHIVEHELPER(unsigned short, int64_t)
		ASL_JSONARCHIVEHELPER(int, int64_t)
		ASL_JSONARCHIVEHELPER(unsigned int, int64_t)
		ASL_JSONARCHIVEHELPER(long, int64_t)
		ASL_JSONARCHIVEHELPER(unsigned long, int64_t)
		ASL_JSONARCHIVEHELPER(long long, int64_t)
		ASL_JSONARCHIVEHELPER(unsigned long long, int64_t)
		ASL_JSONARCHIVEHELPER(float, double)
		ASL_JSONARCHIVEHELPER(double, double)
		//ASL_JSONARCHIVEHELPER(long double, double)
		ASL_JSONARCHIVEHELPER(bool, bool)
		ASL_JSONARCHIVEHELPER(std::string, std::string)

#undef ASL_JSONARCHIVEHELPER
	}
	
	/**
	 * @class JsonArchive
	 * @brief Json存档类
	 */
	class JsonArchive : public Archive {
	protected:
		JsonArchive();
		~JsonArchive();

	public:
		/**
		 * @brief 获取Json文档
		 * @return 返回Json文档
		 */
		JsonNode* GetDoc();

		/**
		 * @brief 分离Json文档
		 * @return 返回Json文档
		 */
		JsonNode* Death();

	protected:
		JsonNode* m_pJsonDoc;							///< Json存档文档
		helper::JsonArchiveIoContext_t m_ctxIoContext;	///< IO上下文
	};

	/**
	 * @class JsonInputArchive
	 * @brief Json输入存档类
	 * @warning 该类所有操作可能抛出ArchiveException异常
	 */
	class JsonInputArchive : public JsonArchive {
	public:
		JsonInputArchive(std::istream& isStream);
		JsonInputArchive(JsonNode* pNode);
		~JsonInputArchive();

	public:
		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const {
			return true;
		}

		template <typename T>
		JsonInputArchive& operator& (const archivable_t<T>& v) {
			helper::JsonArchiveHelper<T>::Parse(m_ctxIoContext, v);
			return *this;
		}
		template <typename T>
		JsonInputArchive& operator>> (const archivable_t<T>& v) {
			helper::JsonArchiveHelper<T>::Parse(m_ctxIoContext, v);
			return *this;
		}

	private:
		std::vector<char> m_acDataBuffer;		///< 输入缓存
	};

	/**
	 * @class JsonOutputArchive
	 * @brief Json输出存档类
	 * @warning 该类所有操作可能抛出ArchiveException异常
	 */
	class JsonOutputArchive : public JsonArchive {
	public:
		JsonOutputArchive(std::ostream& osStream, bool bFormat = false);
		JsonOutputArchive(JsonNode* pNode, bool bFormat = false);
		~JsonOutputArchive();

	public:
		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const {
			return false;
		}

		template <typename T>
		JsonOutputArchive& operator& (const archivable_t<T>& v) {
			helper::JsonArchiveHelper<T>::Serial(m_ctxIoContext, v);
			m_bNeedFlush = true;
			return *this;
		}
		template <typename T>
		JsonOutputArchive& operator<< (const archivable_t<T>& v) {
			helper::JsonArchiveHelper<T>::Serial(m_ctxIoContext, v);
			m_bNeedFlush = true;
			return *this;
		}

		/**
		 * @brief 刷新流
		 */
		void Flush();

	private:
		std::ostream* m_pStream;	///< 输出流
		bool m_bNeedFlush;			///< 是否需要刷新
		bool m_bFormat;				///< 是否格式化输出
	};

	namespace helper {
		/**
		 * @brief Json存档辅助器
		 */
		template <typename M, typename N>
		struct JsonArchiveHelper<std::pair<M, N> > {
			typedef std::pair<M, N> ValueType_t;
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) {
				assert(ctx.pArchive != NULL && ctx.pNode != NULL);
				assert(v.name != NULL);
				JsonNode* pNode = ctx.pNode;
				JsonNode* pChild = new JsonObject();
				if(pChild == NULL) {
					throw ArchiveException("error when alloc json node");
				}
				if(pNode->GetType() == JNT_Array) {
					((JsonArray*)pNode)->AddItem(pChild);
				} else if(pNode->GetType() == JNT_Object) {
					((JsonObject*)pNode)->SetItem(v.name, pChild);
				} else {
					assert(false);
					throw ArchiveException("error when add item");
				}

				ctx.pNode = pChild;
				JsonOutputArchive& a(*(JsonOutputArchive*)ctx.pArchive);
				a & ASL_ARCHIVABLE_NAMEVALUE("Key", v.value.first);
				a & ASL_ARCHIVABLE_NAMEVALUE("Value", v.value.second);
				ctx.pNode = pNode;
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) {
				assert(ctx.pArchive != NULL && ctx.pNode != NULL);
				assert(v.name != NULL);
				JsonNode* pNode = ctx.pNode;
				JsonNode* pChild = NULL;
				if(pNode->GetType() == JNT_Array) {
					assert(ctx.nArrayIndex < ((JsonArray*)pNode)->GetItemCount());
					pChild = ((JsonArray*)pNode)->GetItem(ctx.nArrayIndex);
				} else if(pNode->GetType() == JNT_Object) {
					pChild = ((JsonObject*)pNode)->GetItem(v.name);
				} else {
					assert(false);
					throw ArchiveException("error when get item");
				}
				
				if(pChild != NULL && pChild->GetType() != JNT_Null) {
					ctx.pNode = pChild;
					JsonInputArchive& a(*(JsonInputArchive*)ctx.pArchive);
					a & ASL_ARCHIVABLE_NAMEVALUE("Key", v.value.first);
					a & ASL_ARCHIVABLE_NAMEVALUE("Value", v.value.second);
					ctx.pNode = pNode;
				} else if(!v.maybe) {
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str());
				}
			}
		};
		
#define ASL_JSONARCHIVEHELPER_STDTEMPLATE(T, itemType, paramList, addproc) \
		template <paramList> \
		struct JsonArchiveHelper<T > { \
			typedef T ValueType_t; \
		public: \
			static void Serial(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) { \
				assert(ctx.pArchive != NULL && ctx.pNode != NULL); \
				assert(v.name != NULL); \
				JsonNode* pNode = ctx.pNode; \
				JsonNode* pChild = new JsonArray(); \
				if(pChild == NULL) { \
					throw ArchiveException("error when alloc json node"); \
				} \
				if(pNode->GetType() == JNT_Array) \
				{ \
					((JsonArray*)pNode)->AddItem(pChild); \
				} else if(pNode->GetType() == JNT_Object) { \
					((JsonObject*)pNode)->SetItem(v.name, pChild); \
				} else { \
					assert(false); \
					throw ArchiveException("error when add item"); \
				} \
				\
				ctx.pNode = pChild; \
				JsonOutputArchive& a(*(JsonOutputArchive*)ctx.pArchive); \
				for(typename ValueType_t::iterator iter = v.value.begin(); \
					iter != v.value.end(); ++iter) { \
					itemType item = *iter; \
					a & ASL_ARCHIVABLE_NAMEVALUE("", item); \
				} \
				ctx.pNode = pNode; \
			} \
			static void Parse(JsonArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) { \
				assert(ctx.pArchive != NULL && ctx.pNode != NULL); \
				assert(v.name != NULL); \
				v.value.clear(); \
				JsonNode* pNode = ctx.pNode; \
				JsonNode* pChild = NULL; \
				if(pNode->GetType() == JNT_Array) { \
					assert(ctx.nArrayIndex < ((JsonArray*)pNode)->GetItemCount()); \
					pChild = ((JsonArray*)pNode)->GetItem(ctx.nArrayIndex); \
				} else if(pNode->GetType() == JNT_Object) { \
					pChild = ((JsonObject*)pNode)->GetItem(v.name); \
				} else { \
					assert(false); \
					throw ArchiveException("error when get item"); \
				} \
				\
				if(pChild != NULL && pChild->GetType() != JNT_Null) { \
					ctx.pNode = pChild; \
					JsonInputArchive& a(*(JsonInputArchive*)ctx.pArchive); \
					int nCount = ((JsonArray*)pChild)->GetItemCount(); \
					itemType item; \
					for(int i = 0; i < nCount; ++i) { \
						ctx.nArrayIndex = i; \
						a & ASL_ARCHIVABLE_NAMEVALUE("", item); \
						v.value.addproc(item); \
					} \
					ctx.pNode = pNode; \
				} else if(!v.maybe) { \
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str()); \
				} \
			} \
		}; \

		ASL_JSONARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::vector<N>), ASL_ONEPARAM(N), ASL_ONEPARAM(typename N), push_back)
		ASL_JSONARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::list<N>), ASL_ONEPARAM(N), ASL_ONEPARAM(typename N), push_back)
		ASL_JSONARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::map<M,N>), ASL_ONEPARAM(std::pair<M,N>), ASL_ONEPARAM(typename M,typename N), insert)
#undef ASL_JSONARCHIVEHELPER_STDTEMPLATE
	}
}
