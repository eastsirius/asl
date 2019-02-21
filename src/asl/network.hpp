/**
 * @file network.hpp
 * @brief 网络支持
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "socket.hpp"
#include "time.hpp"
#include "system.hpp"
#include <memory>
#include <functional>
#include <map>
#include <list>

struct addrinfo;

namespace ASL_NAMESPACE {
    /**
     * @brief 网络传输服务
     */
    class NetService : public NoCopyable {
    public:
        NetService();
        ~NetService();

        typedef std::function<void()> Handler_t; ///< 事件句柄
        typedef std::map<SOCKET, Handler_t> HandlerMap_t; ///< 事件句柄影射表
        typedef std::function<void(const addrinfo* pRet)> GetAddrInfoHandler_t; ///< 域名解析结果句柄

        /**
         * @brief 域名解析上下文
         */
        struct GetAddrInfoSession {
            GetAddrInfoSession(const char* szName, const char* szService, const addrinfo* pSrcReq);
            ~GetAddrInfoSession();

            bool bOk;
            Timer trTimer;
            std::string strName;
            std::string strService;
            void* pReq;
            void* pRet;
            GetAddrInfoHandler_t funHandler;
        };

        typedef std::shared_ptr<GetAddrInfoSession> GetAddrInfoSessionPtr_t; ///< 域名解析上下文指针
        typedef std::list<GetAddrInfoSessionPtr_t> GetAddrInfoSessionList_t; ///< 域名解析上下文列表

    public:
        /**
		 * @brief 开启服务
		 * @return 返回执行结果
		 */
        bool Start();

        /**
		 * @brief 停止服务
		 */
        void Stop();

        /**
		 * @brief 添加套接字
		 * @param nSocket 套接字
         * @param funReadHandler 读事件句柄
         * @param funWriteHandler 写事件句柄
		 * @return 返回执行结果
		 */
        bool Add(SOCKET nSocket, Handler_t funReadHandler,
                Handler_t funWriteHandler = Handler_t());

        /**
		 * @brief 编辑套接字
		 * @param nSocket 套接字
         * @param funReadHandler 读事件句柄
         * @param funWriteHandler 写事件句柄
		 * @return 返回执行结果
		 */
        bool Modify(SOCKET nSocket, Handler_t funReadHandler,
                Handler_t funWriteHandler = Handler_t());

        /**
		 * @brief 移除套接字
		 * @param nSocket 套接字
		 */
        void Delete(SOCKET nSocket);

        /**
		 * @brief 运行单次循环
		 * @param nTimeout 毫秒超时
		 */
        void RunOnce(int nTimeout);

        /**
		 * @brief 异步域名解析
		 * @param szName 主机名
         * @param szService 服务名
         * @param pReq 请求参数
         * @param funHandler 结果回调
		 */
        void AsyncGetAddrInfo(const char* szName, const char* szService, const addrinfo* pReq,
                GetAddrInfoHandler_t funHandler);

    private:
        /**
		 * @brief 事件处理
		 * @param nSocket 套接字
         * @param bReadOrWrite 读事件或写事件
		 */
        void _DoEvent(SOCKET nSocket, bool bReadOrWrite);

        /**
		 * @brief 域名解析结果测试
		 */
        void _TestGetAddrInfoResult();

    private:
        int m_nHandle; ///< 事件管理句柄
        std::map<SOCKET, bool> m_mpSocketMap; ///< 套接字列表
        HandlerMap_t m_mpReadHandlerMap;      ///< 读事件句柄影射表
        HandlerMap_t m_mpWriteHandlerMap;     ///< 写事件句柄影射表
        GetAddrInfoSessionList_t m_lstGetAddrInfoSessionList; ///< 域名解析上下文列表
    };

    /**
     * @brief 网络地址
     */
    class NetAddr {
    public:
        NetAddr();
        NetAddr(const NetAddr& naAddr);
        NetAddr(uint32_t dwIP, uint16_t wPort);
        NetAddr(const char* szIP, uint16_t wPort);
        NetAddr(uint16_t wPort);
#ifdef UNIX
        NetAddr(const char* szSocketName); // AF_UNIX 协议栈
#endif
        NetAddr(const sockaddr* pAddr, int nLen);

    public:
        /**
         * @brief 获取socket地址
         * @return 返回socket地址
         */
        sockaddr* GetAddr();

        /**
         * @brief 获取socket地址
         * @return 返回socket地址
         */
        const sockaddr* GetAddr() const;

        /**
         * @brief 获取socket地址长度
         * @return 返回socket地址长度
         */
        int GetAddrLen() const;

        /**
         * @brief 获取最大socket地址长度
         * @return 返回最大socket地址长度
         */
        int GetMaxAddrLen() const;

        /**
         * @brief 获取IP
         * @return 返回网络字节序四字节类型IP
         */
        uint32_t GetIP() const;

        /**
         * @brief 获取网络端口
         * @return 返回本地字节序端口
         */
        uint16_t GetPort() const;

        /**
         * @brief 获取字符串形式IP
         * @return 返回格式化后字符串
         */
        std::string IPToString() const;

        /**
         * @brief 获取字符串形式网络地址
         * @return 返回格式化后字符串
         */
        std::string ToString() const;

    private:
        /**
         * @brief 初始化
         */
        void _Init();

    private:
        struct NetAddrContext_t* m_pContext; ///< socket地址
    };

    /**
     * @class NetSocket
     * @brief 网络套接字
     */
    class NetSocket : public NoCopyable {
    public:
        NetSocket(NetService& nsNetService);
        virtual ~NetSocket();

        typedef std::function<void(NetSocket* pSocket)> ReadEventHandler_t;		///< 读事件处理句柄
        typedef std::function<void(NetSocket* pSocket)> WriteEventHandler_t;	///< 写事件处理句柄
        typedef std::function<void(NetSocket* pSocket, ErrorCode ec)> ConnectEventHandler_t;	///< 连接事件处理句柄

    public:
        /**
         * @brief 获取系统套接字
         * @return 返回系统套接字
         */
        const Socket& GetSocket() const;

        /**
		 * @brief 设置发送缓冲大小
         * @param nSize 缓冲大小
         * @return 返回执行结果
         */
        bool SetSendBufSize(int nSize);

        /**
         * @brief 设置接收缓冲大小
         * @param nSize 缓冲大小
         * @return 返回执行结果
         */
        bool SetRecvBufSize(int nSize);

        /**
         * @brief 获取本地套接字地址
         * @return 返回套接字
         */
        NetAddr GetLocalAddr();

        /**
         * @brief 获取本地套接字地址
         * @return 返回套接字
         */
        NetAddr GetPeerAddr();

        /**
         * @brief 关闭套接字
         */
        void Close();

        /**
         * @brief 读事件处理函数
         */
        void _OnRead();

        /**
         * @brief 写事件处理函数
         */
        void _OnWrite();

    protected:
        /**
         * @brief 获取本地套接字地址
         * @param hSocket 绑定已有系统套接字，INVALID_SOCKET则内部创建
         * @param bStream 是否为流模式
         * @param bAcceptor 是否为TCP连接器
         * @param pAddr 绑定地址，NULL使用随机分配
         * @param bWriteEvent 是否开启写事件
         * @param ec 返回错误码
         * @return 返回执行结果
         */
        bool _CreateSocket(SOCKET hSocket, bool bStream, bool bAcceptor, const NetAddr* pAddr,
                bool bWriteEvent, ErrorCode& ec);

        /**
         * @brief 设置事件处理函数
         * @param funReadEventHandler 读事件处理函数
         * @param funWriteEventHandler 写事件处理函数
         */
        void _SetHandler(ReadEventHandler_t funReadEventHandler, WriteEventHandler_t funWriteEventHandler);

        /**
         * @brief 重置事件选项
         * @param bReadEvent 是否接收读事件
         * @param bWriteEvent 是否接收写事件
         */
        void _ResetEventFlags(bool bReadEvent, bool bWriteEvent);

        /**
         * @brief 连接事件处理函数
         */
        virtual void _OnConnect();

    protected:
        Socket m_hSocket;	///< 系统套接字
        NetService& m_nsNetService;	///< 网络传输服务
        bool m_bConnecting;	///< 是否正在连接
        ReadEventHandler_t m_funReadEventHandler;	///< 读事件处理函数
        WriteEventHandler_t m_funWriteEventHandler;	///< 写事件处理函数
    };

    /**
     * @class UDPSocket
     * @brief UDP套接字
     */
    class UDPSocket : public NetSocket {
    public:
        UDPSocket(NetService& nsNetService, ErrorCode& ec,
                  ReadEventHandler_t funReadEventHandler,
                  WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t());
        UDPSocket(NetService& nsNetService, const NetAddr& naAddr, ErrorCode& ec,
                  ReadEventHandler_t funReadEventHandler,
                  WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t());
        virtual ~UDPSocket();

    public:
        /**
         * @brief 发送数据报
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param naAddr 对端地址
         * @param nTimeout 毫秒超时时间
         * @return 成功返回发送的字节数，超时返回0，失败返回负数
         */
        int SendTo(const uint8_t* pBuf, int nSize, const NetAddr& naAddr, int nTimeout) {
            ErrorCode ec;
            return SendTo(pBuf, nSize, naAddr, ec, nTimeout);
        }

        /**
         * @brief 发送数据报
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param naAddr 对端地址
         * @param ec 错误代码
         * @param nTimeout 毫秒超时时间
         * @return 成功返回发送的字节数，超时返回0，失败返回负数
         */
        int SendTo(const uint8_t* pBuf, int nSize, const NetAddr& naAddr, ErrorCode& ec, int nTimeout);

        /**
         * @brief 接收数据报
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param naAddr 对端地址
         * @return 成功返回接收的字节数，超时返回0，失败返回负数
         */
        int RecvFrom(uint8_t* pBuf, int nSize, NetAddr& naAddr) {
            ErrorCode ec;
            return RecvFrom(pBuf, nSize, naAddr, ec);
        }

        /**
         * @brief 接收数据报
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param naAddr 对端地址
         * @param ec 错误代码
         * @return 成功返回接收的字节数，超时返回0，失败返回负数
         */
        int RecvFrom(uint8_t* pBuf, int nSize, NetAddr& naAddr, ErrorCode& ec);
    };

    /**
     * @class TCPSocket
     * @brief TCP套接字
     */
    class TCPSocket : public NetSocket {
    public:
        TCPSocket(NetService& nsNetService, ErrorCode& ec,
                  ReadEventHandler_t funReadEventHandler,
                  WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t());
        TCPSocket(NetService& nsNetService, SOCKET hSocket, ErrorCode& ec,
                  ReadEventHandler_t funReadEventHandler,
                  WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t());
        TCPSocket(NetService& nsNetService, const NetAddr& naAddr, ErrorCode& ec,
                  ReadEventHandler_t funReadEventHandler,
                  WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t());
        ~TCPSocket();

    public:
        /**
         * @brief 建立连接
         * @param naAddr 对端地址
		 * @param nTimeout 毫秒超时时间
         * @return 返回错误代码
         */
        int Connect(const NetAddr& naAddr, int nTimeout = 30000);

        /**
         * @brief 异步建立连接
         * @param naAddr 对端地址
         * @param funConnectEventHandler 时间回调函数
         */
        void AsyncConnect(const NetAddr& naAddr, ConnectEventHandler_t funConnectEventHandler);

        /**
         * @brief 发送流数据
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param nTimeout 毫秒超时时间
         * @return 成功返回发送的字节数，超时返回0，失败返回负数
         */
        int Send(const uint8_t* pBuf, int nSize, int nTimeout) {
            ErrorCode ec;
            return Send(pBuf, nSize, ec, nTimeout);
        }

        /**
         * @brief 发送流数据
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param ec 错误代码
         * @param nTimeout 毫秒超时时间
         * @return 成功返回发送的字节数，超时返回0，失败返回负数
         */
        int Send(const uint8_t* pBuf, int nSize, ErrorCode& ec, int nTimeout);

        /**
         * @brief 接收流据报
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @return 成功返回接收的字节数，超时返回0，失败返回负数
         */
        int Recv(uint8_t* pBuf, int nSize) {
            ErrorCode ec;
            return Recv(pBuf, nSize, ec);
        }

        /**
         * @brief 接收流据报
         * @param pBuf 数据缓冲
         * @param nSize 缓冲大小
         * @param ec 错误代码
         * @return 成功返回接收的字节数，超时返回0，失败返回负数
         */
        int Recv(uint8_t* pBuf, int nSize, ErrorCode& ec);

    protected:
        /**
         * @brief 连接事件处理函数
         */
        virtual void _OnConnect();

    private:
        ConnectEventHandler_t m_funConnectEventHandler; ///< 连接时间处理句柄
    };

    /**
     * @class TCPAcceptor
     * @brief TCP监听器
     */
    class TCPAcceptor : public NetSocket {
    public:
        TCPAcceptor(NetService& nsNetService, const NetAddr& naAddr, ErrorCode& ec,
                    ReadEventHandler_t funReadEventHandler);
        ~TCPAcceptor();

    public:
        /**
         * @brief 接收连接
         * @param nsNetService 新连接网络传输服务
         * @param funReadEventHandler 新连接读事件处理函数
         * @param funWriteEventHandler 新连接写事件处理函数
         * @return 成功返回新建TCP套接字句柄，失败返回NULL
         */
        TCPSocket* Accept(NetService& nsNetService,
                ReadEventHandler_t funReadEventHandler,
                WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t()) {
            ErrorCode ec;
            return Accept(nsNetService, ec, funReadEventHandler, funWriteEventHandler);
        }

        /**
         * @brief 接收连接
         * @param nsNetService 新连接网络传输服务
         * @param ec 错误代码
         * @param funReadEventHandler 新连接读事件处理函数
         * @param funWriteEventHandler 新连接写事件处理函数
         * @return 成功返回新建TCP套接字句柄，失败返回NULL
         */
        TCPSocket* Accept(NetService& nsNetService, ErrorCode& ec,
                ReadEventHandler_t funReadEventHandler,
                WriteEventHandler_t funWriteEventHandler = WriteEventHandler_t());

        /**
         * @brief 跳过连接，此时低层会接收并关闭连接
         */
        void Skip();

    private:
        /**
         * @brief 接收系统连接套接字
         * @param pEc 错误代码
         * @return 成功返回系统套接字，失败返回INVALID_SOCKET
         */
        SOCKET _DoAccept(ErrorCode& ec);
    };
}
