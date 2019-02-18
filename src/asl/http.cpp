/**
 * @file http.cpp
 * @brief http协议支持
 * @author 程行通
 */

#include "http.hpp"
#include "inter/http_parser.hpp"
#include "time.hpp"
#include <cstring>

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

	std::string HttpBase::URLEncode(const char* szSrc, bool bUpperCase) {
		assert(szSrc != NULL);
		if(szSrc == NULL) {
			return "";
		}

		char baseChar = bUpperCase ? 'A' : 'a';
		std::vector<uint8_t> lstDest;
		uint8_t c;
		uint8_t* pSrc = (uint8_t*)szSrc;
		while(*pSrc) {
			c = *pSrc;
			if(isalpha(c) || isdigit(c) || c == '-' || c == '.' || c == '~') {
				lstDest.push_back(c);
			} else if(c == ' ') {
				lstDest.push_back('+');
			} else {
				lstDest.push_back('%');
				lstDest.push_back(uint8_t((c >= 0xA0) ? ((c >> 4) - 10 + baseChar) : ((c >> 4) + '0')));
				lstDest.push_back(uint8_t(((c & 0xF) >= 0xA) ? ((c & 0xF) - 10 + baseChar) : ((c & 0xF) + '0')));
			}
			++pSrc;
		}
		lstDest.push_back(0);

		return (char*)&lstDest[0];
	}

	std::string HttpBase::URLDecode(const char* szSrc) {
		assert(szSrc != NULL);
		if(szSrc == NULL) {
			return "";
		}

		uint8_t* pSrc = (unsigned char*)szSrc;
		std::vector<uint8_t> lstDest;
		uint8_t c;
		while(*pSrc) {
			if(*pSrc == '%') {
				c = 0;

				//高位
				if(pSrc[1] >= 'A' && pSrc[1] <= 'F') {
					c += (pSrc[1] - 'A' + 10) * 0x10;
				} else if(pSrc[1] >= 'a' && pSrc[1] <= 'f') {
					c += (pSrc[1] - 'a' + 10) * 0x10;
				} else {
					c += (pSrc[1] - '0') * 0x10;
				}

				//低位
				if(pSrc[2] >= 'A' && pSrc[2] <= 'F') {
					c += (pSrc[2] - 'A' + 10);
				} else if(pSrc[2] >= 'a' && pSrc[2] <= 'f') {
					c += (pSrc[2] - 'a' + 10);
				} else {
					c += (pSrc[2] - '0');
				}

				lstDest.push_back(c);
				pSrc += 3;
			} else if(*pSrc == '+') {
				lstDest.push_back(' ');
				++pSrc;
			} else {
				lstDest.push_back(*pSrc);
				++pSrc;
			}
		}

		lstDest.push_back(0);

		return (char*)&lstDest[0];
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


    HttpServer:: HttpServer(NetService& nsNetService, HttpRequestHandler_t funRequestProc)
        : BaseTCPServer(nsNetService), m_funRequestProc(funRequestProc) {
    }

    HttpServer::~HttpServer() {
    }

    int HttpServer::_ParseData(TCPSocket* pSocket, uint8_t* pData, int nSize) {
        HttpRequest req;
        int nRet = req.Parse((char*)pData, nSize);
        if(nRet > 0) {
            if(req.GetMethod() != std::string("POST") && req.GetMethod() != std::string("GET")) {
                return -1;
            }

            auto handler = [this, pSocket](const HttpResponse& hrResp) {
                this->_SendResponse(hrResp, pSocket);
            };
            m_funRequestProc(req, handler);
        }

        return nRet;
    }

    bool HttpServer::_SendResponse(const HttpResponse& hrResp, TCPSocket* pSocket) {
	    Buffer buf(hrResp.GetBodyLength() + 16 * 1024);
	    if(!buf) {
	        return false;
	    }

        int nDataSize = hrResp.Serial((char*)buf.GetBuffer(), buf.GetBufferSize());
        if(nDataSize <= 0) {
            return false;
        }

        return _SendData(pSocket, buf.GetBuffer(), nDataSize, 10000);
    }
}
