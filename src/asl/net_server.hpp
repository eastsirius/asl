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
         * @param pSocket 套接字指针
         * @param pData 数据指针
         * @param nSize 数据长度
         * @return 成功返回以解析长度，失败返回负数，数据不足返回0
         */
        virtual int _ParseData(TCPSocket* pSocket, uint8_t* pData, int nSize) = 0;

        /**
         * @brief 发送数据
         * @param pSocket 套接字
         * @param pData 数据缓冲
         * @param nSize 数据长度
         * @param nTimeout 毫秒超时时间
         * @return 返回执行结果
         */
        bool _SendData(TCPSocket* pSocket, const uint8_t* pData, int nSize, int nTimeout);

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
         * @param pSocket 套接字
         */
        void _Disconnect(TCPSocket* pSocket);

        /**
         * @brief 监听器读事件处理函数
         * @param pSocket 套接字
         */
        void _OnListenerRead(TCPAcceptor* pSocket);

        /**
         * @brief 连接读事件处理函数
         * @param pSocket 套接字
         * @param pSession 连接上下文
         */
        void _OnRead(TCPSocket* pSocket, TCPConnSession* pSession);

    protected:
        NetService& m_nsNetService;     ///< 网络传输服务

        Mutex m_mtxListenersLock;   ///< 监听器哦列表锁
        std::vector<std::shared_ptr<TCPAcceptor> > m_lstListeners;  ///< 监听器列表
        Mutex m_mtxConnectionsLock; ///< 连接列表锁
        std::map<TCPSocket*, TCPConnSessionPtr_t> m_mpConnections;  ///< 连接列表
    };
}
