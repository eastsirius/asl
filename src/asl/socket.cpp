/**
 * @file socket.cpp
 * @brief 套接字
 * @author 程行通
 */

#include "socket.hpp"
#include <cassert>
#ifdef WINDOWS
#  include <Ws2tcpip.h>
#else
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <errno.h>
#endif
#include "time.hpp"

#ifndef WINDOWS
#  define closesocket close
#endif

namespace ASL_NAMESPACE {
	Socket::Socket() : m_hSocket(INVALID_SOCKET) {
	}

	Socket::~Socket() {
		Release();
	}

	bool Socket::Create(int family, int type, int proc) {
		Release();

		m_hSocket = socket(family, type, proc);
		if(IsEmpty()) {
			return false;
		} else {
#ifdef APPLE
			int optval = 1;
			setsockopt(m_hSocket, SOL_SOCKET, SO_NOSIGPIPE, (char*)&optval, sizeof(int));
#endif
			return true;
		}
	}

	void Socket::Release() {
		if(!IsEmpty()) {
			closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}
	}

	void Socket::Attach(SOCKET s) {
		Release();
		m_hSocket = s;
	}

	SOCKET Socket::Death() {
		SOCKET hRet = m_hSocket;
		m_hSocket = INVALID_SOCKET;

		return hRet;
	}

	bool Socket::IsEmpty() {
		return m_hSocket == INVALID_SOCKET;
	}

	bool Socket::Bind(const sockaddr* addr, int addr_len) {
		return bind(m_hSocket, addr, addr_len) == 0;
	}

	bool Socket::Bind(int port, const char* ip) {
		if(IsEmpty() || port == 0) {
			assert(false);
			return false;
		}

		sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		if(ip == NULL) {
			addr.sin_addr.s_addr = INADDR_ANY;
		} else {
			addr.sin_addr.s_addr = inet_addr(ip);
			if(addr.sin_addr.s_addr == INADDR_NONE) {
				return false;
			}
		}
		addr.sin_port = htons(port);

		return Bind((const sockaddr*)&addr, sizeof(addr));
	}

	bool Socket::Bind(const char* addr) {
		std::string src(addr);
		auto pos = src.find_last_of(':');
		if(pos == std::string::npos) {
			return Bind(atoi(addr));
		} else {
			return Bind(atoi(addr + pos + 1), std::string(addr, pos).c_str());
		}
	}

	bool Socket::Listen(int back_log) {
		assert(!IsEmpty());

		return listen(m_hSocket, back_log) == 0;
	}

	SOCKET Socket::Accept() {
		assert(!IsEmpty());

		sockaddr_in addr = {0};
#ifdef WINDOWS
		int nLen = sizeof(addr);
#else
		socklen_t nLen = sizeof(addr);
#endif
		return accept(m_hSocket, (sockaddr*)&addr, &nLen);
	}

	bool Socket::Connect(const sockaddr* addr, int addr_len) {
		assert(addr != NULL);
		assert(!IsEmpty());

		if(connect(m_hSocket, addr, addr_len) == 0) {
			return true;
		}

#ifdef WINDOWS
		if(GetLastError() != WSAEWOULDBLOCK)
#else
		if(errno != EINPROGRESS)
#endif
		{
			return false;
		}

		return true;
	}

	bool Socket::Connect(const char* ip, int port) {
		assert(ip != NULL && port > 0u);

		sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip);
		if(addr.sin_addr.s_addr == INADDR_NONE) {
			return false;
		}
		addr.sin_port = htons(port);

		return Connect((const sockaddr*)&addr, sizeof(addr));
	}

	bool Socket::Connect(const char* addr) {
		std::string src(addr);
		auto pos = src.find_last_of(':');
		if(pos == std::string::npos) {
			return false;
		} else {
			return Connect(std::string(addr, pos).c_str(), atoi(addr + pos + 1));
		}
	}

	bool Socket::TimedConnect(const sockaddr* addr, int addr_len, int timeout) {
		assert(addr != NULL);
		assert(!IsEmpty());

		if(connect(m_hSocket, addr, addr_len) == 0) {
			return true;
		}
#ifdef WINDOWS
		if(GetLastError() != WSAEWOULDBLOCK)
#else
		if(errno != EINPROGRESS)
#endif
		{
			return false;
		}

		if(WaitToWrite(timeout) > 0) {
			return LastSocketError() == 0;
		} else {
			return false;
		}
	}

	bool Socket::TimedConnect(const char* ip, int port, int timeout) {
		assert(ip != NULL && port > 0u);

		sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip);
		if(addr.sin_addr.s_addr == INADDR_NONE) {
			return false;
		}
		addr.sin_port = htons(port);

		return TimedConnect((const sockaddr*)&addr, sizeof(addr), timeout);
	}

	bool Socket::TimedConnect(const char* addr, int timeout) {
		std::string src(addr);
		auto pos = src.find_last_of(':');
		if(pos == std::string::npos) {
			return false;
		} else {
			return TimedConnect(std::string(addr, pos).c_str(), atoi(addr + pos + 1), timeout);
		}
	}

	int Socket::Send(const char* buf, int size) {
		assert(buf != NULL && size > 0);
		assert(!IsEmpty());
#if (defined LINUX && !defined APPLE)
		return send(m_hSocket, buf, size, MSG_NOSIGNAL);
#else
		return send(m_hSocket, buf, size, 0);
#endif
	}

	int Socket::TimedSend(const char* buf, int size, int timeout, bool wait_all) {
		if(timeout == 0) {
			return Send(buf, size);
		}

		int nRet = -1;
		int64_t nTo = 0;
		if(wait_all) {
			if(timeout < 0) {
				timeout = 0x7FFFFFFF;
			}
			int nData = 0;
			bool bFirst = true;
			Timer t;
			while(nData < size && t.MillisecTime() < timeout) {
				if(bFirst) {
					nRet = Send(buf, size);
					bFirst = false;
				} else {
					nRet = WaitToWrite((int)nTo);
					if(nRet < 0) {
						return nRet;
					}
					if(nRet > 0) {
						nRet = Send(buf + nData, size - nData);
					}
				}
				if(nRet > 0) {
					nData += nRet;
				}
			}
			if(nData > 0) {
				nRet = nData;
			}
		} else {
			nRet = Send(buf, size);
			if(nRet <= 0) {
				if(timeout < 0) {
					timeout = 0x7FFFFFFF;
				}
				nRet = WaitToWrite(timeout);
				if(nRet > 0) {
					nRet = Send(buf, size);
				}
			}
		}

		return nRet;
	}

	int Socket::Recv(char* buf, int size) {
		assert(buf != NULL && size > 0);
		assert(!IsEmpty());
#if (defined LINUX && !defined APPLE)
		return recv(m_hSocket, buf, size, MSG_NOSIGNAL);
#else
		return recv(m_hSocket, buf, size, 0);
#endif
	}

	int Socket::TimedRecv(char* buf, int size, int timeout, bool wait_all) {
		if(timeout == 0) {
			return Recv(buf, size);
		}

		int nRet = -1;
		int64_t nTo = 0;
		if(wait_all) {
			if(timeout < 0) {
				timeout = 0x7FFFFFFF;
			}
			int nData = 0;
			bool bFirst = true;
			Timer t;
			while(nData < size && t.MillisecTime() < timeout) {
				if(bFirst) {
					nRet = Recv(buf, size);
					bFirst = false;
				} else {
					nRet = WaitToRead((int)nTo);
					if(nRet < 0) {
						return nRet;
					}
					if(nRet > 0) {
						nRet = Recv(buf + nData, size - nData);
					}
				}
				if(nRet > 0) {
					nData += nRet;
				}
			}
			if(nData > 0) {
				nRet = nData;
			}
		} else {
			nRet = Recv(buf, size);
			if(nRet <= 0) {
				if(timeout < 0) {
					timeout = 0x7FFFFFFF;
				}
				nRet = WaitToRead(timeout);
				if(nRet > 0) {
					nRet = Recv(buf, size);
				}
			}
		}

		return nRet;
	}

	int Socket::SendTo(const char* buf, int size, const sockaddr* addr, int addr_len) {
		assert(buf != NULL && size > 0 && addr != NULL);
		assert(!IsEmpty());
#if (defined LINUX && !defined APPLE)
		return sendto(m_hSocket, buf, size, MSG_NOSIGNAL, addr, addr_len);
#else
		return sendto(m_hSocket, buf, size, 0, addr, addr_len);
#endif
	}

	int Socket::TimedSendTo(const char* buf, int size, const sockaddr* addr, int addr_len, int timeout) {
		int nRet = SendTo(buf, size, addr, addr_len);
		if(nRet <= 0) {
			if(timeout < 0) {
				timeout = 0x7FFFFFFF;
			}

			nRet = WaitToWrite(timeout);
			if(nRet > 0) {
				nRet = SendTo(buf, size, addr, addr_len);
			}
		}

		return nRet;
	}

	int Socket::RecvFrom(char* buf, int size, sockaddr* addr, int& addr_len) {
		assert(buf != NULL && size > 0 && addr != NULL);
		assert(!IsEmpty());
#ifdef WINDOWS
		int nLen = addr_len;
#else
		socklen_t nLen = addr_len;
#endif

#if (defined LINUX && !defined APPLE)
		int nRet = recvfrom(m_hSocket, buf, size, MSG_NOSIGNAL, addr, &nLen);
#else
		int nRet = recvfrom(m_hSocket, buf, size, 0, addr, &nLen);
#endif
		addr_len = nLen;
		return nRet;
	}

	int Socket::TimedRecvFrom(char* buf, int size, sockaddr* addr, int& addr_len, int timeout) {
		int nRet = RecvFrom(buf, size, addr, addr_len);
		if(nRet <= 0) {
			if(timeout < 0) {
				timeout = 0x7FFFFFFF;
			}

			nRet = WaitToRead(timeout);
			if(nRet > 0) {
				nRet = RecvFrom(buf, size, addr, addr_len);
			}
		}

		return nRet;
	}

	bool Socket::SetNoBlockMode(bool mode) {
		assert(!IsEmpty());
#ifdef WINDOWS
		u_long iMode = mode ? 1 : 0;
		return ioctlsocket(m_hSocket, FIONBIO, &iMode) == 0;
#else
		int flags = fcntl(m_hSocket, F_GETFL, 0);
		if(flags == -1) {
			return false;
		}

		if(mode) {
			flags |= O_NONBLOCK;
		} else {
			flags &= ~O_NONBLOCK;
		}

		return fcntl(m_hSocket, F_SETFL, flags) == 0;
#endif
	}

	bool Socket::SetSendBufSize(int size) {
		assert(!IsEmpty());
		return setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size)) == 0;
	}

	bool Socket::SetRecvBufSize(int size) {
		assert(!IsEmpty());
		return setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size)) == 0;
	}

	bool Socket::SetSendTimeout(int timeout) {
		assert(!IsEmpty());
#ifdef WINDOWS
		int nTime = timeout;
		return setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTime, sizeof(nTime)) == 0;
#else
		timeval tv = { int(timeout / 1000), int(timeout % 1000 * 1000) };
		return setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv)) == 0;
#endif
	}

	bool Socket::SetRecvTimeout(int timeout) {
		assert(!IsEmpty());
#ifdef WINDOWS
		int nTime = timeout;
		return setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime)) == 0;
#else
		timeval tv = { int(timeout / 1000), int(timeout % 1000 * 1000) };
		return setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) == 0;
#endif
	}

	bool Socket::SetPortReuse(bool enable) {
		assert(!IsEmpty());
		int nVal = enable ? 1 : 0;
		return setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nVal, sizeof(nVal)) == 0;
	}

	bool Socket::GetLocalAddr(sockaddr* addr, int& addr_len) {
		assert(!IsEmpty());
#ifdef WINDOWS
		int nLen = addr_len;
#else
		socklen_t nLen = addr_len;
#endif
		int nRet = getsockname(m_hSocket, addr, &nLen);
		addr_len = nLen;

		return nRet == 0;
	}

	bool Socket::GetPeerAddr(sockaddr* addr, int& addr_len) {
		assert(!IsEmpty());
#ifdef WINDOWS
		int nLen = addr_len;
#else
		socklen_t nLen = addr_len;
#endif
		int nRet = getpeername(m_hSocket, addr, &nLen);
		addr_len = nLen;

		return nRet == 0;
	}

	int Socket::WaitToRead(int timeout) {
		assert(!IsEmpty());
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(m_hSocket, &readSet);
		timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
		return select((int)m_hSocket + 1, &readSet, NULL, NULL, &tv);
	}

	int Socket::WaitToWrite(int timeout) {
		assert(!IsEmpty());
		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(m_hSocket, &writeSet);
		timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
		return select((int)m_hSocket + 1, NULL, &writeSet, NULL, &tv);
	}

	int Socket::LastSocketError() {
		int error = 0;
		socklen_t len = sizeof(error);
		if(getsockopt(m_hSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0) {
			return -1;
		}
		return error;
	}

	Socket::operator SOCKET() const {
		return m_hSocket;
	}

	bool Socket::InitSDK() {
#ifdef WINDOWS
		WSADATA wsaData = {0};
		return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
		return true;
#endif
	}

	void Socket::ReleaseSDK() {
#ifdef WINDOWS
		WSACleanup();
#endif
	}
}
