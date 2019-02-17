/**
 * @file socket.hpp
 * @brief 套接字
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#ifdef WINDOWS
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#endif

#ifndef WINDOWS
#  define SOCKET int
#  define INVALID_SOCKET -1
#endif

namespace ASL_NAMESPACE {
	/**
	 * @class Socket
	 * @brief 网络套接字
	 */
	class Socket : public NoCopyable {
	public:
		Socket();
		~Socket();

	public:
		/**
		 * @brief 创建套接字
		 * @param family 协议簇
		 * @param type 套接字类型
		 * @param proc 协议
		 * @return 返回执行结果
		 */
		bool Create(int family = AF_INET, int type = SOCK_STREAM, int proc = 0);

		/**
		 * @brief 释放套接字
		 */
		void Release();

		/**
		 * @brief 绑定套接字
		 * @param s 待绑定套接字
		 */
		void Attach(SOCKET s);

		/**
		 * @brief 分离套接字
		 * @return 返回分离的套接字
		 */
		SOCKET Death();

		/**
		 * @brief 判断套接字是否为空
		 * @return 返回套接字是否为空
		 */
		bool IsEmpty();

		/**
		 * @brief 绑定地址
		 * @param addr 对端地址指针
		 * @param addr_len 对端地址大小
		 * @return 返回执行结果
		 */
		bool Bind(const sockaddr* addr, int addr_len);

		/**
		 * @brief 绑定地址
		 * @param port 端口号
		 * @param ip IP
		 * @return 返回执行结果
		 */
		bool Bind(int port, const char* ip = 0);

		/**
		 * @brief 开始监听
		 * @param back_log 队列长度
		 * @return 返回执行结果
		 */
		bool Listen(int back_log);

		/**
		 * @brief 接受连接
		 * @return 新接受的连接套接字
		 */
		SOCKET Accept();

		/**
		 * @brief 建立连接
		 * @param addr 对端地址指针
		 * @param addr_len 对端地址大小
		 * @return 返回执行结果
		 */
		bool Connect(const sockaddr* addr, int addr_len);

		/**
		 * @brief 建立连接
		 * @param ip 对端IP
		 * @param port 对端端口
		 * @return 返回执行结果
		 */
		bool Connect(const char* ip, int port);

		/**
		 * @brief 带计时的建立连接(非阻塞模式下有效)
		 * @param addr 对端地址指针
		 * @param addr_len 对端地址大小
		 * @param timeout 毫秒超时时间
		 * @return 返回执行结果
		 */
		bool TimedConnect(const sockaddr* addr, int addr_len, int timeout);

		/**
		 * @brief 带计时的建立连接(非阻塞模式下有效)
		 * @param ip 对端IP
		 * @param port 对端端口
		 * @param timeout 毫秒超时时间
		 * @return 返回执行结果
		 */
		bool TimedConnect(const char* ip, int port, int timeout);

		/**
		 * @brief 发送流数据
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @return 成功返回发送的字节数，出错返回负数
		 */
		int Send(const char* buf, int size);

		/**
		 * @brief 带计时的发送流数据(非阻塞模式下有效)
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @param timeout 毫秒超时时间
		 * @param wait_all 是否等待全部完成
		 * @return 成功返回发送的字节数，出错返回负数
		 */
		int TimedSend(const char* buf, int size, int timeout, bool wait_all = true);

		/**
		 * @brief 接收流数据
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @return 成功返回接收的字节数，出错返回负数
		 */
		int Recv(char* buf, int size);

		/**
		 * @brief 带计时的接收流数据(非阻塞模式下有效)
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @param timeout 毫秒超时时间
		 * @param wait_all 是否等待全部完成
		 * @return 成功返回接收的字节数，出错返回负数
		 */
		int TimedRecv(char* buf, int size, int timeout, bool wait_all = false);

		/**
		 * @brief 发送数据报
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @param addr 对端地址
		 * @param addr_len 对端地址大小
		 * @return 成功返回发送的字节数，出错返回负数
		 */
		int SendTo(const char* buf, int size, const sockaddr* addr, int addr_len);

		/**
		 * @brief 带计时的发送数据报(非阻塞模式下有效)
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @param addr 对端地址
		 * @param addr_len 对端地址大小
		 * @param timeout 毫秒超时时间
		 * @return 成功返回发送的字节数，出错返回负数
		 */
		int TimedSendTo(const char* buf, int size, const sockaddr* addr, int addr_len, int timeout);

		/**
		 * @brief 接收数据报
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @param addr 对端地址
		 * @param addr_len 对端地址大小
		 * @return 成功返回接收的字节数，出错返回负数
		 */
		int RecvFrom(char* buf, int size, sockaddr* addr, int& addr_len);

		/**
		 * @brief 带计时的接收数据报(非阻塞模式下有效)
		 * @param buf 数据缓冲指针
		 * @param size 数据缓冲大小
		 * @param addr 对端地址
		 * @param addr_len 对端地址大小
		 * @param timeout 毫秒超时时间
		 * @return 成功返回接收的字节数，出错返回负数
		 */
		int TimedRecvFrom(char* buf, int size, sockaddr* addr, int& addr_len, int timeout);

		/**
		 * @brief 设置异步模式
		 * @param mode 是否为异步模式
		 * @return 返回执行结果
		 */
		bool SetNoBlockMode(bool mode);

		/**
		 * @brief 设置发送缓冲
		 * @param size 发送缓冲大小
		 * @return 返回执行结果
		 */
		bool SetSendBufSize(int size);

		/**
		 * @brief 设置接收缓冲
		 * @param size 接收缓冲大小
		 * @return 返回执行结果
		 */
		bool SetRecvBufSize(int size);

		/**
		 * @brief 设置发送超时时间
		 * @param timeout 发送超时毫秒时间
		 * @return 返回执行结果
		 */
		bool SetSendTimeout(int timeout);

		/**
		 * @brief 设置接收超时时间
		 * @param timeout 接收超时毫秒时间
		 * @return 返回执行结果
		 */
		bool SetRecvTimeout(int timeout);

		/**
		 * @brief 设置端口复用
		 * @param enable 是否开启端口复用
		 * @return 返回执行结果
		 */
		bool SetPortReuse(bool enable);

		/**
		 * @brief 获取本地地址
		 * @param addr 返回地址
		 * @param addr_len 返回地址长度
		 * @return 返回执行结果
		 */
		bool GetLocalAddr(sockaddr* addr, int& addr_len);

		/**
		 * @brief 获取对端地址
		 * @param addr 返回地址
		 * @param addr_len 返回地址长度
		 * @return 返回执行结果
		 */
		bool GetPeerAddr(sockaddr* addr, int& addr_len);

		/**
		 * @brief 等待读取
		 * @param timeout 毫秒超时时间
		 * @return 成功返回正数，失败返回负数，超时返回0
		 */
		int WaitToRead(int timeout);

		/**
		 * @brief 等待写入
		 * @param timeout 毫秒超时时间
		 * @return 成功返回正数，失败返回负数，超时返回0
		 */
		int WaitToWrite(int timeout);

		/**
		 * @brief 获取套接字最后错误
		 * @return 返回错误码
		 */
		int LastSocketError();

		operator SOCKET() const;

	public:
		/**
		 * @brief 初始化全局资源
		 * @return 返回执行结果
		 */
		static bool InitSDK();

		/**
		 * @brief 释放全局资源
		 */
		static void ReleaseSDK();

	private:
		SOCKET m_hSocket;				///< 套接字句柄
	};
}
