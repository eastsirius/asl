/**
 * @file http.cpp
 * @brief http协议支持
 * @author 程行通
 */

#include "http.hpp"
#include "inter/http_parser.hpp"
#include "time.hpp"
#include "url.hpp"
#include <cstring>
#include <regex>

namespace ASL_NAMESPACE {
    /**
     * @brief HTTP辅助类
     */
	class HttpHelper {
    public:
        HttpHelper(HttpBase& hbHttp) : m_hbHttp(hbHttp) {
        }

    public:
        /**
         * @brief 初始化
         * @param hpParser 解析器
         * @param stSettings 解析配置
         * @param bOnlyHeader 仅解析HTTP头
         */
        void Init(http_parser& hpParser, http_parser_settings& stSettings, bool bOnlyHeader) {
            m_bOnlyHeader = bOnlyHeader;
            m_bParseOK = false;

            hpParser.data = this;

            stSettings.on_message_begin    = NULL;
			stSettings.on_url              = OnUrl;
			stSettings.on_status_complete  = OnStatusComplete;
			stSettings.on_header_field     = OnHeaderField;
			stSettings.on_header_value     = OnHeaderValue;
			stSettings.on_headers_complete = OnHeadersComplete;
			stSettings.on_body             = OnBody;
            stSettings.on_message_complete = OnMessageComplete;
		}

        /**
         * @brief 判断是否解析完成
         * @param hpParser 解析器
         * @param stSettings 解析配置
         * @return 返回是否解析完成
         */
        bool IsParseOK() {
            return m_bParseOK;
        }

        /**
         * @brief URL处理函数
         * @param pParser 解析器
         * @param pData 数据缓冲
         * @param sLen 数据大小
         * @return 继续解析返回0，结束解析返回非0
         */
		static int OnUrl(http_parser* pParser, const char* pData, size_t sLen) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
			std::string strUrl(pData, sLen);
            pHelper->m_hbHttp._OnUrl(strUrl.c_str());
			return 0;
		}

        /**
         * @brief 状态行解析完成处理函数
         * @param pParser 解析器
         * @return 继续解析返回0，结束解析返回非0
         */
		static int OnStatusComplete(http_parser* pParser) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
            pHelper->m_hbHttp._OnState(pParser->status_code);
			return 0;
		}

        /**
         * @brief 头部域名处理函数
         * @param pParser 解析器
         * @param pData 数据缓冲
         * @param sLen 数据大小
         * @return 继续解析返回0，结束解析返回非0
         */
		static int OnHeaderField(http_parser* pParser, const char* pData, size_t sLen) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
            pHelper->m_strTempField = std::string(pData, sLen);
			return 0;
		}

        /**
         * @brief 头部域值处理函数
         * @param pParser 解析器
         * @param pData 数据缓冲
         * @param sLen 数据大小
         * @return 继续解析返回0，结束解析返回非0
         */
		static int OnHeaderValue(http_parser* pParser, const char* pData, size_t sLen) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
			std::string strValue(pData, sLen);
            pHelper->m_hbHttp.SetHeaderField(pHelper->m_strTempField.c_str(), strValue.c_str());
			return 0;
		}

        /**
         * @brief HTTP头解析完成处理函数
         * @param pParser 解析器
         * @return 继续解析返回0，结束解析返回非0
         */
		static int OnHeadersComplete(http_parser* pParser) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
            if(pHelper->m_bOnlyHeader)
            {
                pHelper->m_hbHttp.SetBody(NULL, (int)pParser->content_length);
                pHelper->m_bParseOK = true;
                return 1;
            }

			return 0;
		}

        /**
         * @brief 数据区处理函数
         * @param pParser 解析器
         * @param pData 数据缓冲
         * @param sLen 数据大小
         * @return 继续解析返回0，结束解析返回非0
         */
		static int OnBody(http_parser* pParser, const char* pData, size_t sLen) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
            pHelper->m_hbHttp.SetBody(pData, sLen);
			return 0;
		}

        /**
         * @brief HTTP包解析完成处理函数
         * @param pParser 解析器
         * @param pData 数据缓冲
         * @param sLen 数据大小
         * @return 继续解析返回0，结束解析返回非0
         */
        static int OnMessageComplete(http_parser* pParser) {
            HttpHelper* pHelper = (HttpHelper*)pParser->data;
            pHelper->m_bParseOK = true;
			return 1;
		}

        /**
         * @brief HTTP状态值转换为字符串
         * @param nState 状态值
         * @return 返回字符串形式状态
         */
        static const char* HttpStateToString(int nState) {
            static struct {
                const int nState;
                const char* szMsg;
            } s_mpStateMsgMap[] = {
                { 200, "OK" },
                { 201, "Created" },
                { 202, "Accepted" },
                { 203, "Non-Authoritative Information" },
                { 204, "No Content" },
                { 205, "Reset Content" },
                { 206, "Partial Content" },
                { 207, "Multi-Status" },
                { 300, "Multiple Choices" },
                { 301, "Moved Permanently" },
                { 302, "Move temporarily" },
                { 303, "See Other" },
                { 304, "Not Modified" },
                { 305, "Use Proxy" },
                { 306, "Switch Proxy" },
                { 307, "Temporary Redirect" },
                { 400, "Bad Request" },
                { 401, "Unauthorized" },
                { 402, "Payment Required" },
                { 403, "Forbidden" },
                { 404, "Not Found" },
                { 405, "Method Not Allowed" },
                { 406, "Not Acceptable" },
                { 407, "Proxy Authentication Required" },
                { 408, "Request Timeout" },
                { 409, "Conflict" },
                { 410, "Gone" },
                { 411, "Length Required" },
                { 412, "Precondition Failed" },
                { 413, "Request Entity Too Large" },
                { 414, "Request-URI Too Long" },
                { 415, "Unsupported Media Type" },
                { 416, "Requested Range Not Satisfiable" },
                { 417, "Expectation Failed" },
                { 421, "There are too many connections from your internet address" },
                { 422, "Unprocessable Entity" },
                { 423, "Locked" },
                { 424, "Failed Dependency" },
                { 425, "Unordered Collection" },
                { 426, "Upgrade Required" },
                { 449, "Retry With" },
                { 500, "Internal Server Error" },
                { 501, "Not Implemented" },
                { 502, "Bad Gateway" },
                { 503, "Service Unavailable" },
                { 504, "Gateway Timeout" },
                { 505, "HTTP Version Not Supported" },
                { 506, "Variant Also Negotiates" },
                { 507, "Insufficient Storage" },
                { 509, "Bandwidth Limit Exceeded" },
                { 510, "Not Extended" },
                { 600, "Unparseable Response Headers" },
                { -1, "Unknown" }
            };

            for(int i = 0; true; ++i) {
                if(s_mpStateMsgMap[i].nState == nState || s_mpStateMsgMap[i].nState == -1) {
                    return s_mpStateMsgMap[i].szMsg;
                }
            }
            return "";
        }

    private:
        HttpBase& m_hbHttp;         ///< HTTP类
        bool m_bOnlyHeader;         ///< 是否仅解析HTTP头
        bool m_bParseOK;            ///< 是否解析完成
        std::string m_strTempField; ///< 最后头部域名
    };


	HttpBase::HttpBase() : m_pBody(NULL), m_nBodyLen(0) {
	}

    int HttpBase::Parse(const char* szData, int nLen, bool bOnlyHeader) {
        m_pBody = NULL;
        m_nBodyLen = 0;
        m_mpHeaderValueMap.clear();

        HttpHelper helper(*this);
        http_parser parser = {0};
        http_parser_settings settings = {0};
        helper.Init(parser, settings, bOnlyHeader);
        http_parser_init(&parser, IsRequest() ? HTTP_REQUEST : HTTP_RESPONSE);

        size_t ret = http_parser_execute(&parser, &settings, szData, nLen);
        if(helper.IsParseOK()) {
            _OnMethod(http_method_str((http_method)parser.method));
        }

        return helper.IsParseOK() ? ret : ((int)ret == nLen ? 0 : -1);
    }

    int HttpBase::Serial(char* pBuf, int nSize, int nBodyLen) const {
        int nFree = nSize;
        const char* pBody = NULL;
        if(nBodyLen < 0) {
            pBody = m_pBody;
			nBodyLen = m_nBodyLen;
        }
		if(nBodyLen > 0) {
			assert(pBody != NULL);
            ((HttpBase*)this)->SetHeaderField("Content-Length", std::to_string(nBodyLen).c_str());
		}

#define SUL_WRITEDATA(buf, size) \
        if(int(size) > nFree) { \
            return -1; \
        } \
        memcpy(pBuf, buf, size); \
        pBuf += size; \
        nFree -= size; \

        std::string strTemp = _SerialHeaderLine();
        SUL_WRITEDATA(strTemp.c_str(), strTemp.length());

        for(int i = 0; i < GetHeaderFieldCount(); ++i) {
            strTemp = GetHeaderFieldName(i);
            strTemp += ": ";
            strTemp += GetHeaderFieldValue(i);
            strTemp += "\r\n";
            SUL_WRITEDATA(strTemp.c_str(), strTemp.length());
        }
        SUL_WRITEDATA("\r\n", 2);
        if(pBody != NULL && nBodyLen > 0) {
            SUL_WRITEDATA(pBody, nBodyLen);
        }
#undef SUL_WRITEDATA

        return nSize - nFree;
    }

	std::string HttpBase::UrlEncode(const char* szSrc, bool bUpperCase) {
		return Url::UrlEncode(szSrc, bUpperCase);
	}

	std::string HttpBase::UrlDecode(const char* szSrc) {
		return Url::UrlDecode(szSrc);
	}

    int HttpBase::GetHeaderFieldCount() const {
        return m_mpHeaderValueMap.size();
    }

    const char* HttpBase::GetHeaderFieldValue(const char* szField) const {
        for(size_t i = 0; i < m_mpHeaderValueMap.size(); ++i) {
            if(strcasecmp(m_mpHeaderValueMap[i].first.c_str(), szField) == 0) {
                return m_mpHeaderValueMap[i].second.c_str();
            }
        }
        return "";
    }

    const char* HttpBase::GetHeaderFieldName(int nIndex) const {
        if(nIndex < 0 || nIndex > (int)m_mpHeaderValueMap.size()) {
            return "";
        }
        return m_mpHeaderValueMap[nIndex].first.c_str();
    }

    const char* HttpBase::GetHeaderFieldValue(int nIndex) const {
        if(nIndex < 0 || nIndex > (int)m_mpHeaderValueMap.size()) {
            return "";
        }
        return m_mpHeaderValueMap[nIndex].second.c_str();
    }

    void HttpBase::SetHeaderField(const char* szField, const char* szValue) {
        for(size_t i = 0; i < m_mpHeaderValueMap.size(); ++i) {
            if(strcasecmp(m_mpHeaderValueMap[i].first.c_str(), szField) == 0) {
                if(szValue == NULL) {
                    m_mpHeaderValueMap.erase(m_mpHeaderValueMap.begin() + i);
                } else {
                    m_mpHeaderValueMap[i].second = szValue;
                }
                return;
            }
        }
        m_mpHeaderValueMap.push_back(std::pair<std::string, std::string>(szField, szValue));
    }

    const char* HttpBase::GetTimeField() const {
        return GetHeaderFieldValue("Date");
    }

    void HttpBase::SetTimeField() {
        static const char* s_szWeeks[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        static const char* s_szMouths[] =
            {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        Datetime t = Datetime::GetSystemTime();
        char acBuffer[32];
        sprintf(acBuffer, "%s, %.2d %s %d %.2d:%.2d:%.2d GMT", s_szWeeks[t.GetDayOfWeek() - 1],
            (int)t.GetDay(), s_szMouths[t.GetMonth() - 1], (int)t.GetYear(),
            (int)t.GetHour(), (int)t.GetMinute(), (int)t.GetDay());

        SetHeaderField("Date", acBuffer);
    }

    bool HttpBase::GetConnectionKeepAlive() const {
        std::string strValue = GetHeaderFieldValue("Connection");
        if(strValue.empty()) {
            return true;
        }
        return strcasecmp(strValue.c_str(), "Keep-Alive") == 0;
    }

    void HttpBase::SetConnectionField(bool bKeepAlive) {
        SetHeaderField("Connection", bKeepAlive ? "Keep-Alive" : "Close");
    }

    void HttpBase::_OnMethod(const char*) {
	}

    void HttpBase::_OnUrl(const char*) {
	}

    void HttpBase::_OnState(int) {
	}


    HttpRequest::HttpRequest() {
    }

    std::string HttpRequest::_SerialHeaderLine() const {
        return std::string(GetMethod()) + " " + GetUrl() + " HTTP/1.1\r\n";
    }


    HttpResponse::HttpResponse() : m_nStateCode(200) {
    }

    const char* HttpResponse::GetStateString() const {
        return HttpHelper::HttpStateToString(GetState());
    }

    std::string HttpResponse::_SerialHeaderLine() const {
        std::string strTemp = "HTTP/1.1 ";
        strTemp += std::to_string(m_nStateCode);
        strTemp += " ";
        strTemp += GetStateString();
        strTemp += "\r\n";
        return strTemp;
    }


    FunctionHttpHandler::FunctionHttpHandler(HttpRequestHandler_t funRequestHandler)
            : m_funRequestHandler(funRequestHandler) {
	}

    FunctionHttpHandler::~FunctionHttpHandler() {
	}

	void FunctionHttpHandler::HttpProc(const HttpRequest& hrReq, WriteRespHandler_t funWriteResp) {
        m_funRequestHandler(hrReq, funWriteResp);
	}


	HttpMux::HttpMux() {
        ClearHandler();
	}

	HttpMux::~HttpMux() {
	}

	void HttpMux::HttpProc(const HttpRequest& hrReq, WriteRespHandler_t funWriteResp) {
	    for(auto iter = m_lstHandlerSessions.begin(); iter != m_lstHandlerSessions.end(); ++iter) {
	        if(_MatchUrl(iter->mtType, iter->strKey, hrReq.GetUrl())) {
	            if(iter->pHandler) {
	                iter->pHandler->HttpProc(hrReq, funWriteResp);
	            }
	            return;
	        }
	    }

	    if(m_p404Handler) {
	        m_p404Handler->HttpProc(hrReq, funWriteResp);
	    }
	}

	void HttpMux::AddHandler(MatchType mtType, std::string strKey, HttpHandlerPtr_t pHandler) {
        HandlerSession session;
        session.mtType = mtType;
        session.strKey = strKey;
        session.pHandler = pHandler;
        m_lstHandlerSessions.push_back(session);
	}

	void HttpMux::ClearHandler() {
	    m_p404Handler = std::make_shared<FunctionHttpHandler>(_Default404Proc);
        m_lstHandlerSessions.clear();
	}

    bool HttpMux::_MatchUrl(MatchType mtType, const std::string& strKey, const std::string& strUrl) {
	    switch(mtType) {
	        case MT_FullString:
	            return strKey == strUrl;
	            break;
	        case MT_MatchBegin:
	            return strncmp(strKey.c_str(), strUrl.c_str(), strKey.length()) == 0;
	            break;
	        case MT_Regex: {
	            std::regex re(strKey);
	            std::cmatch m;
	            return std::regex_match(strUrl.c_str(), m, re);
                break;
	        }
	        default:
	            break;
	    }

	    return true;
	}

    void HttpMux::_Default404Proc(const HttpRequest& hrReq, WriteRespHandler_t funWriteResp) {
        HttpResponse resp;
        resp.SetState(404);
        resp.SetTimeField();
        funWriteResp(resp);
	}


    HttpServer:: HttpServer(NetService& nsNetService, HttpHandlerPtr_t pHandler)
        : BaseTCPServer(nsNetService), m_pHandler(pHandler) {
    }

    HttpServer::~HttpServer() {
    }

    int HttpServer::_ParseData(int64_t nConnId, uint8_t* pData, int nSize) {
        HttpRequest req;
        int nRet = req.Parse((char*)pData, nSize);
        if(nRet > 0) {
            if(req.GetMethod() != std::string("POST") && req.GetMethod() != std::string("GET")) {
                return -1;
            }

            auto handler = [this, nConnId](const HttpResponse& hrResp) {
                this->_SendResponse(nConnId, hrResp);
            };
            m_pHandler->HttpProc(req, handler);
        }

        return nRet;
    }

    bool HttpServer::_SendResponse(int64_t nConnId, const HttpResponse& hrResp) {
	    Buffer buf(hrResp.GetBodyLength() + 16 * 1024);
	    if(!buf) {
	        return false;
	    }

        int nDataSize = hrResp.Serial((char*)buf.GetBuffer(), buf.GetBufferSize());
        if(nDataSize <= 0) {
            return false;
        }

        return _SendData(nConnId, buf.GetBuffer(), nDataSize, 1000);
    }


    HttpClient::HttpClient() {
	}

    HttpClient::~HttpClient() {
	    Close();
	}

    void HttpClient::Close() {
	    if(m_pClient) {
	        m_pClient->Close();
	        m_pClient.reset();
	    }
	}

    HttpClientPtr_t HttpClient::AsyncCall(NetService& nsNetService, const char* szAddr,
            const HttpRequest& hrReq, ResponseHandler_t funHandler, int nTimeout) {
	    auto pClient = std::make_shared<HttpClient>();
	    if(!pClient->_AsyncCall(nsNetService, szAddr, hrReq, funHandler, nTimeout)) {
	        return NULL;
	    }

	    return pClient;
	}

    HttpClientPtr_t HttpClient::AsyncCall2(NetService& nsNetService, const char* szMethod, const char* szUrl,
	        ResponseHandler_t funHandler, const char* pBody, int nBodySize, int nTimeout) {
        Url url(szUrl);
        if(url.IsEmpty()) {
            funHandler(HttpResponse(), AslError(AECV_Error));
            return NULL;
        }

        HttpRequest req;
        req.SetMethod(szMethod);
        req.SetUrl(url.GetPath().c_str());
        if(pBody != NULL && nBodySize > 0) {
            req.SetBody(pBody, nBodySize);
        }

        std::string strAddr = url.GetHost() + std::to_string(url.GetPort());
        return AsyncCall(nsNetService, strAddr.c_str(), req, funHandler, nTimeout);
    }

    bool HttpClient::_AsyncCall(NetService& nsNetService, const char* szAddr,
            const HttpRequest& hrReq, ResponseHandler_t funHandler, int nTimeout) {
        std::string strUrl = std::string("http://") + szAddr + "/index.html";
        Url url(strUrl.c_str());
        if(url.IsEmpty()) {
            funHandler(HttpResponse(), AslError(AECV_ParamError));
            return NULL;
        }

        Buffer buf(hrReq.GetBodyLength() + 16 * 1024);
        if(!buf) {
            funHandler(HttpResponse(), AslError(AECV_AllocMemoryFailed));
            return NULL;
        }

        int ret = hrReq.Serial((char*)buf.GetBuffer(), buf.GetBufferSize());
        if(ret <= 0) {
            funHandler(HttpResponse(), AslError(AECV_SerialFailed));
            return NULL;
        }

        NetAddr addr(url.GetHost().c_str(), url.GetPort());
        m_pClient = TcpRpcClient::AsyncCall(nsNetService, addr, buf.GetBuffer(), ret, nTimeout,
                [this, funHandler](const uint8_t* pData, int nSize, ErrorCode ec){
            return _OnResponseData(pData, nSize, ec, funHandler);
        });
        if(!m_pClient) {
            return false;
        }

        return true;
	}

    bool HttpClient::_OnResponseData(const uint8_t* pData, int nSize, ErrorCode ec, ResponseHandler_t funHandler) {
        HttpResponse hrResp;

	    if(ec) {
	        funHandler(hrResp, ec);
	        return true;
	    }

	    int ret = hrResp.Parse((const char*)pData, nSize);
	    if(ret < 0) {
            funHandler(hrResp, AslError(AECV_ParseFailed));
            return true;
	    } else if(ret == 0) {
	        return false;
	    }

	    funHandler(hrResp, ErrorCode());

	    return true;
	}
}
