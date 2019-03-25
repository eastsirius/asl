/**
 * @file net_server.hpp
 * @brief 网络服务
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "thread.hpp"
#include "socket.hpp"
#include "memory.hpp"
#include "network.hpp"
#include <atomic>

namespace ASL_NAMESPACE {
    /**
     * @brief 服务类
     */
    class NetServer : public NoCopyable {
    public:
        /**
         * @brief 服务类配置
         */
        class NetServerParam {
        public:
            virtual ~NetServerParam(){}
        };

        NetServer();
        virtual ~NetServer();

    public:
        /**
         * @brief 开启服务
         * @param pParam 服务配置
         * @return 返回执行结果
         */
        virtual bool Start(const NetServerParam* pParam) = 0;

        /**
         * @brief 停止服务
         */
        virtual void Stop() = 0;
    };

    /**
     * @brief TCP服务基类
     */
    class BaseTCPServer : public NetServer {
    public:
        BaseTCPServer(NetService& nsNetService);
        virtual ~BaseTCPServer();

        /**
         * @brief TCP服务基类配置
         */
        class BaseTCPServerParam : public NetServerParam {
        public:
            std::vector<std::string> lstListeners;	///< 监听端口，元素为 [Port] 或 [IP:Port] 形式的字符串
        };

        /**
         * @brief 连接上下文
         */
        struct TCPConnSession {
            Mutex mtxLock;             ///< 同步锁
            TCPSocketPtr_t pSocket;    ///< 套接字
            GrowthBuffer bfRecvBuffer; ///< 接收缓冲
        };
        typedef std::shared_ptr<TCPConnSession> TCPConnSessionPtr_t; ///< 连接上下文指针类型

    public:
        /**
         * @brief 开启服务
         * @param pParam 服务配置
         * @return 返回执行结果
         */
        virtual bool Start(const NetServerParam* pParam);

        /**
         * @brief 停止服务
         */
        virtual void Stop();

    protected:
        /**
         * @brief 数据解析函数
         * @param nConnId 连接ID
         * @param pData 数据指针
         * @param nSize 数据长度
         * @return 成功返回以解析长度，失败返回负数，数据不足返回0
         */
        virtual int _ParseData(int64_t nConnId, uint8_t* pData, int nSize) = 0;

        /**
         * @brief 发送数据
         * @param nConnId 连接ID
         * @param pData 数据缓冲
         * @param nSize 数据长度
         * @param nTimeout 毫秒超时时间
         * @return 返回执行结果
         */
        bool _SendData(int64_t nConnId, const uint8_t* pData, int nSize, int nTimeout);

        /**
         * @brief 创建监听器
         * @param pParam 监听器配置
         * @return 返回执行结果
         */
        bool _CreateListeners(const BaseTCPServerParam* pParam);

        /**
         * @brief 释放监听器
         */
        void _ReleaseListeners();

        /**
         * @brief 断开连接
         * @param nConnId 连接ID
         */
        void _Disconnect(int64_t nConnId);

        /**
         * @brief 监听器读事件处理函数
         * @param pSocket 套接字
         */
        void _OnListenerRead(TCPAcceptor* pSocket);

        /**
         * @brief 连接读事件处理函数
         * @param nConnId 连接ID
         */
        void _OnRead(int64_t nConnId);

        /**
         * @brief 获取连接上下文
         * @param nConnId 连接ID
         * @param bDelete 是否从列表中删除
         */
        TCPConnSessionPtr_t _GetSession(int64_t nConnId, bool bDelete);

    protected:
        NetService& m_nsNetService;     ///< 网络传输服务

        std::atomic_int_fast64_t m_nConnIdCount;    ///< ID计数器
        Mutex m_mtxListenersLock;   ///< 监听器列表锁
        std::vector<std::shared_ptr<TCPAcceptor> > m_lstListeners;  ///< 监听器列表
        Mutex m_mtxConnectionsLock; ///< 连接列表锁
        std::map<int64_t, TCPConnSessionPtr_t> m_mpConnections;  ///< 连接列表
    };

    ASL_SHAREDPTR_PRE_DEF(TcpRpcClient);

    /**
     * @brief TCP RPC客户端
     */
    class TcpRpcClient : public NoCopyable {
    public:
        TcpRpcClient();
        ~TcpRpcClient();

        typedef std::function<bool(const uint8_t* pData, int nSize, ErrorCode ec)> ResponseHandler_t;

    public:
        /**
         * @brief 关闭客户端
         */
        void Close();

        /**
         * @brief 异步调用
         * @param nsNetService 绑定传输服务
         * @param szAddr 服务地址
         * @param pData 请求数据
         * @param nSize 请求数据长度
         * @param nTimeout 毫秒超时时间
         * @param funHandler 结果回调
         * @return 成功返回客户端实例，失败返回空指针
         */
        static TcpRpcClientPtr_t AsyncCall(NetService& nsNetService, const NetAddr& naAddr,
                const uint8_t* pData, int nSize, int nTimeout, ResponseHandler_t funHandler);

    private:
        /**
         * @brief 异步调用
         * @param nsNetService 绑定传输服务
         * @param szAddr 服务地址
         * @param pData 请求数据
         * @param nSize 请求数据长度
         * @param nTimeout 毫秒超时时间
         * @param funHandler 结果回调
         * @return 返回执行结果
         */
        bool _AsyncCall(NetService& nsNetService, const NetAddr& naAddr, const uint8_t* pData,
                int nSize, int nTimeout, ResponseHandler_t funHandler);

        /**
         * @brief 连接事件处理函数
         * @param ec 错误码
         */
        void _OnConnect(ErrorCode ec);

        /**
         * @brief 读取事件处理函数
         */
        void _OnRead();

        /**
         * @brief 写入事件处理函数
         */
        void _OnWrite();

        /**
         * @brief 错误处理
         * @param ec 错误码
         */
        void _DoError(ErrorCode ec);

        /**
         * @brief 发送操作
         */
        void _DoSend();

    private:
        Buffer m_bfSendBuf;             ///< 发送缓存
        GrowthBuffer m_bfRecvBuf;       ///< 接收缓存
        ResponseHandler_t m_funHandler; ///< 响应处理函数
        TCPSocketPtr_t m_pSocket;       ///< 套接字
    };
}
