/**
* @file xmlarchive.hpp
* @brief XML存档工具
* @author 程行通
*/

#pragma once

#include "asldef.hpp"
#include "serialize.hpp"
#include "convert.hpp"
#include "rapidxml/rapidxml.hpp"
#include <vector>
#include <map>
#include <list>

namespace ASL_NAMESPACE {
	class XmlInputArchive;
	class XmlOutputArchive;
	namespace helper {
		/**
		 * @brief XML存档IO上下文
		 */
		struct XmlArchiveIoContext_t {
			Archive* pArchive;						///< 存档器指针
			rapidxml::xml_document<char>* pDoc;		///< XML文档类指针
			rapidxml::xml_node<char>* pNode;		///< 当前XML节点
		};

		/**
		 * @brief XML存档辅助器
		 */
		template <typename T>
		struct XmlArchiveHelper {
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<T>& v) {
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL);
				rapidxml::xml_node<char>* pNode = ctx.pNode;
				rapidxml::xml_node<char>* pChild = ctx.pDoc->allocate_node(rapidxml::node_element, ctx.pDoc->allocate_string(v.name));
				if(pChild == NULL) {
					throw ArchiveException("error when alloc xml node");
				}
				pNode->append_node(pChild);

				AslArchiveSerializer<XmlOutputArchive, T> serializer;
				ctx.pNode = pChild;
				serializer.Serialize(*(XmlOutputArchive*)ctx.pArchive, v.value);
				ctx.pNode = pNode;
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<T>& v) {
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL);
				rapidxml::xml_node<char>* pNode = ctx.pNode;
				rapidxml::xml_node<char>* pChild = pNode->first_node(v.name);
				if(pChild != NULL) {
					ctx.pNode = pChild;
					AslArchiveSerializer<XmlInputArchive, T> serializer;
					serializer.Serialize(*(XmlInputArchive*)ctx.pArchive, v.value);
					ctx.pNode = pNode;
				} else if(!v.maybe) {
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str());
				}
			}
		};

#define ASL_XMLARCHIVEHELPER(type) \
		template <> \
		struct XmlArchiveHelper<type> { \
		public: \
			static void Serial(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<type>& v) { \
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL); \
				assert(v.name != NULL); \
				rapidxml::xml_node<char> *pChild = ctx.pDoc->allocate_node(rapidxml::node_element, \
					ctx.pDoc->allocate_string(v.name), ctx.pDoc->allocate_string(to_string_cast<type>(v.value))); \
				if(pChild == NULL) { \
					throw ArchiveException("error when alloc xml node"); \
				} \
				ctx.pNode->append_node(pChild); \
			} \
			static void Parse(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<type>& v) { \
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL); \
				assert(v.name != NULL); \
				rapidxml::xml_node<char>* pChild = ctx.pNode->first_node(v.name); \
				if(pChild != NULL) { \
					v.value = from_string_cast<type>(pChild->value()); \
				} else if(!v.maybe) { \
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str()); \
				} \
			} \
		}; \

		ASL_XMLARCHIVEHELPER(char)
		ASL_XMLARCHIVEHELPER(signed char)
		ASL_XMLARCHIVEHELPER(unsigned char)
		ASL_XMLARCHIVEHELPER(short)
		ASL_XMLARCHIVEHELPER(unsigned short)
		ASL_XMLARCHIVEHELPER(int)
		ASL_XMLARCHIVEHELPER(unsigned int)
		ASL_XMLARCHIVEHELPER(long)
		ASL_XMLARCHIVEHELPER(unsigned long)
		ASL_XMLARCHIVEHELPER(long long)
		ASL_XMLARCHIVEHELPER(unsigned long long)
		ASL_XMLARCHIVEHELPER(float)
		ASL_XMLARCHIVEHELPER(double)
		//ASL_XMLARCHIVEHELPER(long double)
		ASL_XMLARCHIVEHELPER(bool)
		ASL_XMLARCHIVEHELPER(std::string)

#undef ASL_XMLARCHIVEHELPER
	}

	/**
	 * @class XmlArchive
	 * @brief XML存档类
	 */
	class XmlArchive : public Archive {
	protected:
		XmlArchive();

	protected:
		rapidxml::xml_document<char> m_xdDoc;			///< XML存档文档
		helper::XmlArchiveIoContext_t m_ctxIoContext;	///< IO上下文
	};

	/**
	 * @class XmlInputArchive
	 * @brief XML输入存档类
	 * @warning 该类所有操作可能抛出ArchiveException异常
	 */
	class XmlInputArchive : public XmlArchive {
	public:
		XmlInputArchive(std::istream& isStream);
		~XmlInputArchive();

	public:
		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const {
			return true;
		}

		template <typename T>
		XmlInputArchive& operator& (const archivable_t<T>& v) {
			helper::XmlArchiveHelper<T>::Parse(m_ctxIoContext, v);
			return *this;
		}
		template <typename T>
		XmlInputArchive& operator>> (const archivable_t<T>& v) {
			helper::XmlArchiveHelper<T>::Parse(m_ctxIoContext, v);
			return *this;
		}

	private:
		std::istream& m_isStream;				///< 输入流
		std::vector<char> m_acDataBuffer;		///< 输入缓存
	};

	/**
	 * @class XmlOutputArchive
	 * @brief XML输出存档类
	 * @warning 该类所有操作可能抛出ArchiveException异常
	 */
	class XmlOutputArchive : public XmlArchive {
	public:
		XmlOutputArchive(std::ostream& osStream);
		~XmlOutputArchive();

	public:
		/**
		 * @brief 判断读操作或写操作
		 * @return 读操作返回true，写操作返回false
		 */
		virtual bool IsReadOrWrite() const {
			return false;
		}

		template <typename T>
		XmlOutputArchive& operator& (const archivable_t<T>& v) {
			helper::XmlArchiveHelper<T>::Serial(m_ctxIoContext, v);
			m_bNeedFlush = true;
			return *this;
		}
		template <typename T>
		XmlOutputArchive& operator<< (const archivable_t<T>& v) {
			helper::XmlArchiveHelper<T>::Serial(m_ctxIoContext, v);
			m_bNeedFlush = true;
			return *this;
		}

		/**
		 * @brief 刷新流
		 */
		void Flush();

	private:
		std::ostream& m_osStream;	///< 输出流
		bool m_bNeedFlush;			///< 是否需要刷新
	};

	namespace helper {
		/**
		 * @brief XML存档辅助器
		 */
		template <typename M, typename N>
		struct XmlArchiveHelper<std::pair<M, N> > {
			typedef std::pair<M, N> ValueType_t;
		public:
			/**
			 * @brief 序列化字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Serial(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) {
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL);
				rapidxml::xml_node<char>* pNode = ctx.pNode;
				rapidxml::xml_node<char>* pChild = ctx.pDoc->allocate_node(rapidxml::node_element, ctx.pDoc->allocate_string(v.name));
				if(pChild == NULL) {
					throw ArchiveException("error when alloc xml node");
				}
				pNode->append_node(pChild);

				ctx.pNode = pChild;
				XmlOutputArchive& a(*(XmlOutputArchive*)ctx.pArchive);
				a & ASL_ARCHIVABLE_NAMEVALUE("Key", v.value.first);
				a & ASL_ARCHIVABLE_NAMEVALUE("Value", v.value.second);
				ctx.pNode = pNode;
			}
			
			/**
			 * @brief 解析字段
			 * @param ctx IO上下文
			 * @param v 字段
			 */
			static void Parse(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) {
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL);
				rapidxml::xml_node<char>* pNode = ctx.pNode;
				rapidxml::xml_node<char>* pChild = pNode->first_node(v.name);
				if(pChild != NULL) {
					ctx.pNode = pChild;
					XmlInputArchive& a(*(XmlInputArchive*)ctx.pArchive);
					a & ASL_ARCHIVABLE_NAMEVALUE("Key", v.value.first);
					a & ASL_ARCHIVABLE_NAMEVALUE("Value", v.value.second);
					ctx.pNode = pNode;
				} else if(!v.maybe) {
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str());
				}
			}
		};

#define ASL_XMLARCHIVEHELPER_STDTEMPLATE(type, itemType, paramList, addproc) \
		template <paramList> \
		struct XmlArchiveHelper<type > { \
			typedef type ValueType_t; \
		public: \
			static void Serial(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) { \
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL); \
				rapidxml::xml_node<char>* pNode = ctx.pNode; \
				rapidxml::xml_node<char>* pChild = ctx.pDoc->allocate_node(rapidxml::node_element, ctx.pDoc->allocate_string(v.name)); \
				if(pChild == NULL) { \
					throw ArchiveException("error when alloc xml node"); \
				} \
				pNode->append_node(pChild); \
				\
				ctx.pNode = pChild; \
				XmlOutputArchive& a(*(XmlOutputArchive*)ctx.pArchive); \
				int nCount = (int)v.value.size(); \
				a & ASL_ARCHIVABLE_NAMEVALUE("Count", nCount); \
				int nIndex = 0; \
				char acBuffer[32]; \
				for(typename ValueType_t::iterator iter = v.value.begin(); \
					iter != v.value.end(); ++iter) { \
					sprintf(acBuffer, "Item%d", ++nIndex); \
					itemType item = *iter; \
					a & ASL_ARCHIVABLE_NAMEVALUE(acBuffer, item); \
				} \
				ctx.pNode = pNode; \
			} \
			static void Parse(XmlArchiveIoContext_t& ctx, const Archive::archivable_t<ValueType_t>& v) { \
				assert(ctx.pArchive != NULL && ctx.pDoc != NULL && ctx.pNode != NULL); \
				v.value.clear(); \
				rapidxml::xml_node<char>* pNode = ctx.pNode; \
				rapidxml::xml_node<char>* pChild = pNode->first_node(v.name); \
				if(pChild != NULL) { \
					ctx.pNode = pChild; \
					XmlInputArchive& a(*(XmlInputArchive*)ctx.pArchive); \
					int nCount = 0; \
					a & ASL_ARCHIVABLE_NAMEVALUE("Count", nCount); \
					itemType item; \
					char acBuffer[32]; \
					for(int i = 0; i < nCount; ++i) { \
						sprintf(acBuffer, "Item%d", i + 1); \
						a & ASL_ARCHIVABLE_NAMEVALUE(acBuffer, item); \
						v.value.addproc(item); \
					} \
					ctx.pNode = pNode; \
				} else if(!v.maybe) { \
					throw ArchiveException((std::string("cann't find node ") + v.name).c_str()); \
				} \
			} \
		}; \

		ASL_XMLARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::vector<N>), ASL_ONEPARAM(N), ASL_ONEPARAM(typename N), push_back)
		ASL_XMLARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::list<N>), ASL_ONEPARAM(N), ASL_ONEPARAM(typename N), push_back)
		ASL_XMLARCHIVEHELPER_STDTEMPLATE(ASL_ONEPARAM(std::map<M,N>), ASL_ONEPARAM(std::pair<M,N>), ASL_ONEPARAM(typename M,typename N), insert)
#undef ASL_XMLARCHIVEHELPER_STDTEMPLATE
	}
}
