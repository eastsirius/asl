/**
 * @file http.hpp
 * @brief http协议支持
 * @author 程行通
 */

#pragma once

#include "utils.hpp"
#include "net_server.hpp"
#include <string>
#include <vector>
#include <cassert>

namespace ASL_NAMESPACE {
    /**
     * @brief HTTP包基类
     */
	class HttpBase {
	public:
		HttpBase();

        typedef std::vector<std::pair<std::string, std::string> > HeaderValueMap_t; ///< 头部域值集类型

    public:
        /**
         * @brief 解析HTTP包
         * @param szData 数据缓冲
         * @param nLen 数据长度
         * @param bOnlyHeader 仅解析HTTP头
         * @return 成功返回以解析长度，失败返回负数，数据不足返回0
         */
        int Parse(const char* szData, int nLen, bool bOnlyHeader = false);

        /**
         * @brief 序列化HTTP包
         * @param pBuf 数据缓冲
         * @param nSize 缓冲长度
         * @param nBodyLen 数据区长度，小于0时使用已设置数据区，大于等于0时仅写入HTTP头(包含Content-Length字段)
         * @return 成功返回以解析长度，失败返回负数，数据不足返回0
         */
        int Serial(char* pBuf, int nSize, int nBodyLen = -1) const;

		/**
		 * @brief URL编码
		 * @param szSrc 源字符串
		 * @return 返回编码后字符串
		 */
		static std::string URLEncode(const char* szSrc, bool bUpperCase = true);

		/**
		 * @brief URL解码
		 * @param szSrc 源字符串
		 * @return 返回解码后字符串
		 */
		static std::string URLDecode(const char* szSrc);

    public:
        /**
         * @brief 获取数据区指针
         * @return 返回数据区指针
         */
		const char* GetBody() const {
			return m_pBody;
		}

        /**
         * @brief 获取数据区长度
         * @return 返回数据区长度
         */
        int GetBodyLength() const {
			return m_nBodyLen;
		}

        /**
         * @brief 设置数据区
         * @param szBody 数据区指针
         * @param nBodyLen 数据区长度
         */
        void SetBody(const char* szBody, int nBodyLen) {
            m_pBody = szBody;
            m_nBodyLen = nBodyLen;
        }

        /**
         * @brief 获取头部域计数
         * @return 返回头部域数目
         */
        int GetHeaderFieldCount() const;

        /**
         * @brief 获取头部域值
         * @param szField 域名
         * @return 成功返回域值，失败返回空字符串
         */
        const char* GetHeaderFieldValue(const char* szField) const;

        /**
         * @brief 获取头部域名
         * @param nIndex 索引值
         * @return 成功返回域名，失败返回空字符串
         */
        const char* GetHeaderFieldName(int nIndex) const;

        /**
         * @brief 获取头部域值
         * @param nIndex 索引值
         * @return 成功返回域值，失败返回空字符串
         */
        const char* GetHeaderFieldValue(int nIndex) const;

        /**
         * @brief 设置头部域值
         * @param szField 域名
         * @param szValue 域值(为NULL则删除该头部域)
         */
        void SetHeaderField(const char* szField, const char* szValue);

        /**
         * @brief 获取时间域
         * @return 返回时间域
         */
        const char* GetTimeField() const;

        /**
         * @brief 设置时间域
         */
        void SetTimeField();

        /**
         * @brief 获取连接类型是否为长连接
         * @return 返回连接类型是否为长连接
         */
        bool GetConnectionKeepAlive() const;

        /**
         * @brief 设置连接类型
         * @param bKeepAlive 是否为长连接
         */
        void SetConnectionField(bool bKeepAlive);

        /**
         * @brief 判断是否为请求包
         * @return 返回是否为请求包
         */
        virtual bool IsRequest() const = 0;

        /**
         * @brief 过程名解析相应函数
         * @param szMethod 过程名
         */
        virtual void _OnMethod(const char* szMethod);

        /**
         * @brief URL解析相应函数
         * @param szUrl URL
         */
        virtual void _OnUrl(const char* szUrl);

        /**
         * @brief HTTP状态码解析相应函数
         * @param nState HTTP状态码
         */
        virtual void _OnState(int nState);

    protected:
        /**
         * @brief 序列化HTTP头头行
         * @return 返回HTTP头头行
         */
        virtual std::string _SerialHeaderLine() const = 0;

    private:
        const char* m_pBody;    ///< Body区指针
        int m_nBodyLen;         ///< Body区长度
        HeaderValueMap_t m_mpHeaderValueMap; ///< 头部域值集
	};

    /**
     * @brief HTTP请求类
     */
	class HttpRequest : public HttpBase {
    public:
        HttpRequest();

    public:
        /**
         * @brief 判断是否为请求包
         * @return 返回是否为请求包
         */
        virtual bool IsRequest() const {
            return true;
        }

        /**
         * @brief 获取调用名
         * @return 返回调用名
         */
        const char* GetMethod() const {
            return m_strMethod.c_str();
        }

        /**
         * @brief 设置调用名
         * @param szMethod 调用名
         */
        void SetMethod(const char* szMethod) {
            m_strMethod = szMethod;
        }

        /**
         * @brief 获取URL
         * @return 返回URL
         */
        const char* GetUrl() const {
            return m_strUrl.c_str();
        }

        /**
         * @brief 设置URL
         * @param szUrl URL
         */
        void SetUrl(const char* szUrl) {
            m_strUrl = szUrl;
        }

        /**
         * @brief 过程名解析相应函数
         * @param szMethod 过程名
         */
        virtual void _OnMethod(const char* szMethod) {
            SetMethod(szMethod);
        }

        /**
         * @brief URL解析相应函数
         * @param szUrl URL
         */
        virtual void _OnUrl(const char* szUrl) {
            SetUrl(szUrl);
        }

    protected:
        /**
         * @brief 序列化HTTP头头行
         * @return 返回HTTP头头行
         */
        virtual std::string _SerialHeaderLine() const;

	private:
        std::string m_strMethod;    ///< 调用名
        std::string m_strUrl;       ///< URL
	};

    /**
     * @brief HTTP响应类
     */
	class HttpResponse : public HttpBase {
    public:
        HttpResponse();

    public:
        /**
         * @brief 判断是否为请求包
         * @return 返回是否为请求包
         */
        virtual bool IsRequest() const {
            return false;
        }

        /**
         * @brief 获取HTTP状态码
         * @return 返回HTTP状态码
         */
        int GetState() const {
            return m_nStateCode;
        }

        /**
         * @brief 获取HTTP状态字符串
         * @return 返回HTTP状态字符串
         */
        const char* GetStateString() const;

        /**
         * @brief 设置HTTP状态码
         * @param nState HTTP状态码
         */
        void SetState(int nState) {
            m_nStateCode = nState;
        }

        /**
         * @brief HTTP状态码解析相应函数
         * @param nState HTTP状态码
         */
        virtual void _OnState(int nState) {
            SetState(nState);
        }

	protected:
        /**
         * @brief 序列化HTTP头头行
         * @return 返回HTTP头头行
         */
        virtual std::string _SerialHeaderLine() const;

	private:
        int m_nStateCode; ///< HTTP状态码
	};

    /**
     * @class HttpServer
     * @brief Http服务类
     */
    class HttpServer : public BaseTCPServer {
    public:
        /**
         * @brief TCP服务基类配置
         */
        class HttpServerParam : public BaseTCPServerParam {
        public:
        };

        typedef std::function<void(const HttpResponse& hrResp)> WriteRespHandler_t;
        typedef std::function<void(const HttpRequest& hrReq, WriteRespHandler_t funWriteResp)> HttpRequestHandler_t;

        HttpServer(NetService& nsNetService, HttpRequestHandler_t funRequestProc);
        virtual ~HttpServer();

    protected:
        /**
         * @brief 数据解析函数
         * @param pSocket 套接字指针
         * @param pData 数据指针
         * @param nSize 数据长度
         * @return 成功返回以解析长度，失败返回负数，数据不足返回0
         */
        virtual int _ParseData(TCPSocket* pSocket, uint8_t* pData, int nSize);

        /**
         * @brief 发送响应包
		 * @param hrResp 响应包
         * @param pSocket 套接字指针
         * @return 返回执行结果
         */
        bool _SendResponse(const HttpResponse& hrResp, TCPSocket* pSocket);

    protected:
        HttpRequestHandler_t m_funRequestProc;
    };
}
