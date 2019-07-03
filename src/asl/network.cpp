/**
 * @file network.cpp
 * @brief 网络支持
 * @author 程行通
 */

#include "network.hpp"
#include "thread.hpp"
#include "system.hpp"
#include <cassert>
#include <cstring>
#ifdef WINDOWS
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#endif

#ifdef WINDOWS
#  define ASL_NETSERVICE_USE_SELECT 1
#  define ASL_NETSERVICE_USE_EPOLL 0
#endif

#ifdef LINUX
#  define ASL_NETSERVICE_USE_SELECT 0
#  define ASL_NETSERVICE_USE_EPOLL 1
#  include <sys/un.h>
#  include <arpa/inet.h>
#endif

#if ASL_NETSERVICE_USE_EPOLL
#  define ASL_NETSERVICE_EPOLL_READ_EVENT (EPOLLIN | EPOLLERR)
#  define ASL_NETSERVICE_EPOLL_WRITE_EVENT (EPOLLOUT)
#  include <unistd.h>
#  include <sys/epoll.h>
#endif

namespace ASL_NAMESPACE {
    NetService::GetAddrInfoSession::GetAddrInfoSession(const char* szName, const char* szService,
            const addrinfo* pSrcReq) : bOk(false), strName(szName), strService(szService), pReq(NULL), pRet(NULL) {
        pReq = malloc(sizeof(addrinfo));
        memcpy(pReq, pSrcReq, sizeof(addrinfo));
    }

    NetService::GetAddrInfoSession::~GetAddrInfoSession() {
        if(pReq) {
            free(pReq);
        }
        if(pRet) {
            freeaddrinfo((addrinfo*)pRet);
        }
    }


    NetService::NetService() : m_nHandle(-1), m_nTimerIdCnt(0) {
    }

    NetService::~NetService() {
    }

    bool NetService::Start() {
#if ASL_NETSERVICE_USE_EPOLL
        m_nHandle = epoll_create(0xFFFF);
        if(m_nHandle == -1) {
            return false;
        }
#endif
        return true;
    }

    void NetService::Stop() {
        m_mpSocketMap.clear();
        m_mpReadHandlerMap.clear();
        m_mpWriteHandlerMap.clear();

#if ASL_NETSERVICE_USE_EPOLL
        if(m_nHandle != -1) {
            close(m_nHandle);
            m_nHandle = -1;
        }
#endif
    }

    bool NetService::Add(SOCKET nSocket, Handler_t funReadHandler, Handler_t funWriteHandler) {
        if(m_mpSocketMap.find(nSocket) != m_mpSocketMap.end()) {
            return false;
        }
        if(!funReadHandler && !funWriteHandler) {
            return false;
        }

#if ASL_NETSERVICE_USE_SELECT
        if(m_mpSocketMap.size() >= FD_SETSIZE) {
            return false;
        }
#endif

#if ASL_NETSERVICE_USE_EPOLL
        assert(m_nHandle != -1);
        epoll_event evt;
        memset(&evt, 0, sizeof(evt));
        if(funReadHandler) {
            evt.events |= ASL_NETSERVICE_EPOLL_READ_EVENT;
        }
        if(funWriteHandler) {
            evt.events |= ASL_NETSERVICE_EPOLL_WRITE_EVENT;
        }
        evt.data.fd = nSocket;
        int ret = epoll_ctl(m_nHandle, EPOLL_CTL_ADD, nSocket, &evt);
        if(ret < 0) {
            return false;
        }
#endif

        m_mpSocketMap[nSocket] = true;
        if(funReadHandler) {
            m_mpReadHandlerMap[nSocket] = funReadHandler;
        }
        if(funWriteHandler) {
            m_mpWriteHandlerMap[nSocket] = funWriteHandler;
        }

        return true;
    }

    bool NetService::Modify(SOCKET nSocket, Handler_t funReadHandler, Handler_t funWriteHandler) {
        if(m_mpSocketMap.find(nSocket) == m_mpSocketMap.end()) {
            return false;
        }
        if(!funReadHandler && !funWriteHandler) {
            return false;
        }

#if ASL_NETSERVICE_USE_EPOLL
        assert(m_nHandle != -1);
        epoll_event evt;
        memset(&evt, 0, sizeof(evt));
        if(funReadHandler) {
            evt.events |= ASL_NETSERVICE_EPOLL_READ_EVENT;
        }
        if(funWriteHandler) {
            evt.events |= ASL_NETSERVICE_EPOLL_WRITE_EVENT;
        }
        evt.data.fd = nSocket;
        int ret = epoll_ctl(m_nHandle, EPOLL_CTL_MOD, nSocket, &evt);
        if(ret < 0) {
            return false;
        }
#endif

        m_mpReadHandlerMap.erase(nSocket);
        m_mpWriteHandlerMap.erase(nSocket);
        if(funReadHandler) {
            m_mpReadHandlerMap[nSocket] = funReadHandler;
        }
        if(funWriteHandler) {
            m_mpWriteHandlerMap[nSocket] = funWriteHandler;
        }

        return true;
    }

    void NetService::Delete(SOCKET nSocket) {
        m_mpSocketMap.erase(nSocket);
        m_mpReadHandlerMap.erase(nSocket);
        m_mpWriteHandlerMap.erase(nSocket);
        DeleteSocketTimer(nSocket);

#if ASL_NETSERVICE_USE_EPOLL
        assert(m_nHandle != -1);
        epoll_ctl(m_nHandle, EPOLL_CTL_DEL, nSocket, NULL);
#endif
    }

    uint64_t NetService::AddTimer(int nTimout, Handler_t funHandler) {
        return AddTimer(INVALID_SOCKET, nTimout, funHandler);
    }

    uint64_t NetService::AddTimer(SOCKET nSocket, int nTimout, Handler_t funHandler) {
        TimerSession session;
        session.u64Id = ++m_nTimerIdCnt;
        session.hSocket = nSocket;
        session.nTimeoutTime = asl_get_ms_time() + nTimout;
        session.funHandler = funHandler;
        m_lstTimerSessionList.push_back(session);

        return session.u64Id;
    }

    void NetService::DeleteTimer(uint64_t u64Id) {
        for(auto iter = m_lstTimerSessionList.begin(); iter != m_lstTimerSessionList.end(); ++iter) {
            if(iter->u64Id == u64Id) {
                m_lstTimerSessionList.erase(iter);
                return;
            }
        }
    }

    void NetService::DeleteSocketTimer(SOCKET nSocket) {
        for(auto iter = m_lstTimerSessionList.begin(); iter != m_lstTimerSessionList.end();) {
            if(iter->hSocket != INVALID_SOCKET && iter->hSocket == nSocket) {
                iter = m_lstTimerSessionList.erase(iter);
                continue;
            }
            ++iter;
        }
    }

    void NetService::RunOnce(int nTimeout) {
        _TestTimer();
        _TestGetAddrInfoResult();

        if(m_mpSocketMap.empty()) {
            Thread::Sleep(nTimeout);
            return;
        }

        int ret = -1;
#if ASL_NETSERVICE_USE_SELECT
        fd_set readSet, writeSet;
        int max_fd = -1;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        for(auto iter = m_mpReadHandlerMap.begin(); iter != m_mpReadHandlerMap.end(); ++iter) {
            FD_SET(iter->first, &readSet);
			max_fd = asl_max(max_fd, (int)iter->first);
        }
        for(auto iter = m_mpWriteHandlerMap.begin(); iter != m_mpWriteHandlerMap.end(); ++iter) {
            FD_SET(iter->first, &writeSet);
            max_fd = asl_max(max_fd, (int)iter->first);
        }

        timeval tv = {0};
        tv.tv_sec = nTimeout / 1000;
        tv.tv_usec = nTimeout % 1000 * 1000;
        ret = select(max_fd, &readSet, &writeSet, NULL, &tv);
        if(ret < 0) {
            Thread::Sleep(nTimeout);
        } else if(ret > 0) {
			auto readMap = m_mpReadHandlerMap;
			auto writeMap = m_mpWriteHandlerMap;
            for(auto iter = readMap.begin(); iter != readMap.end(); ++iter) {
                if(FD_ISSET(iter->first, &readSet)) {
                    _DoEvent(iter->first, true);
                }
            }
            for(auto iter = writeMap.begin(); iter != writeMap.end(); ++iter) {
                if(FD_ISSET(iter->first, &writeSet)) {
                    _DoEvent(iter->first, false);
                }
            }
        }
#endif

#if ASL_NETSERVICE_USE_EPOLL
        assert(m_nHandle != -1);
        const int evt_num = 64;
        epoll_event evts[evt_num];
        memset(evts, 0, sizeof(evts));
        ret = epoll_wait(m_nHandle, evts, evt_num, nTimeout);
        if(ret < 0) {
            Thread::Sleep(nTimeout);
        } else if(ret > 0) {
            for(int i = 0; i < ret; ++i) {
                epoll_event& evt(evts[i]);
                if(evt.events & ASL_NETSERVICE_EPOLL_READ_EVENT) {
                    _DoEvent(evt.data.fd, true);
                }
                if(evt.events & ASL_NETSERVICE_EPOLL_WRITE_EVENT) {
                    _DoEvent(evt.data.fd, false);
                }
            }
        }
#endif
    }

    void NetService::AsyncGetAddrInfo(const char* szName, const char* szService, const addrinfo* pReq,
            GetAddrInfoHandler_t funHandler) {
        auto pSession = std::make_shared<GetAddrInfoSession>(szName, szService, pReq);
        pSession->funHandler = funHandler;
        m_lstGetAddrInfoSessionList.push_back(pSession);

        auto funThreadHandler = [pSession]() {
            addrinfo* pRet = NULL;
            if(getaddrinfo(pSession->strName.c_str(), pSession->strName.c_str(),
                    (addrinfo*)pSession->pReq, &pRet) == 0) {
                pSession->pRet = pRet;
            }
            pSession->bOk = true;
        };
        auto pThread = new std::thread(funThreadHandler);
        pThread->detach();
        delete pThread;
    }

    void NetService::_DoEvent(SOCKET nSocket, bool bReadOrWrite) {
        HandlerMap_t& hmap(bReadOrWrite ? m_mpReadHandlerMap : m_mpWriteHandlerMap);
        auto iter = hmap.find(nSocket);
        if(iter != hmap.end()) {
            iter->second();
        }
    }

    void NetService::_TestTimer() {
        std::vector<Handler_t> handlers;
        int64_t cur_time = asl_get_ms_time();
        for(auto iter = m_lstTimerSessionList.begin(); iter != m_lstTimerSessionList.end();) {
            if(cur_time > iter->nTimeoutTime) {
                handlers.push_back(iter->funHandler);
                iter = m_lstTimerSessionList.erase(iter);
                continue;
            }
            ++iter;
        }

        for(auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
            (*iter)();
        }
    }

    void NetService::_TestGetAddrInfoResult() {
        for(auto iter = m_lstGetAddrInfoSessionList.begin();
            iter != m_lstGetAddrInfoSessionList.end();) {
            auto pSession = *iter;
            if(pSession->pRet) {
                pSession->funHandler((addrinfo*)pSession->pRet);
                iter = m_lstGetAddrInfoSessionList.erase(iter);
            } else if(pSession->trTimer.MillisecTime() > 30000) {
                pSession->funHandler(NULL);
                iter = m_lstGetAddrInfoSessionList.erase(iter);
            } else {
                ++iter;
            }
        }
    }


    struct NetAddrContext_t {
        union
        {
            sockaddr addr;
            sockaddr_in addr_in;
#ifdef UNIX
            sockaddr_un addr_un;
#endif
        };	///< socket地址
    };


    NetAddr::NetAddr() {
        _Init();
    }

    NetAddr::NetAddr(const NetAddr& naAddr) {
        _Init();
        *this = naAddr;
    }

    NetAddr::NetAddr(uint32_t dwIP, uint16_t wPort) {
        _Init();
        m_pContext->addr_in.sin_family = AF_INET;
        m_pContext->addr_in.sin_addr.s_addr = dwIP;
        m_pContext->addr_in.sin_port = htons(wPort);
    }

    NetAddr::NetAddr(const char* szIP, uint16_t wPort) {
        _Init();
        m_pContext->addr_in.sin_family = AF_INET;
        m_pContext->addr_in.sin_addr.s_addr = inet_addr(szIP);
        m_pContext->addr_in.sin_port = htons(wPort);
    }

    NetAddr::NetAddr(uint16_t wPort) {
        _Init();
        m_pContext->addr_in.sin_family = AF_INET;
        m_pContext->addr_in.sin_addr.s_addr = INADDR_ANY;
        m_pContext->addr_in.sin_port = htons(wPort);
    }
#ifdef UNIX
    NetAddr::NetAddr(const char* szSocketName) {
        _Init();
        m_pContext->addr_un.sun_family = AF_UNIX;
        snprintf(m_pContext->addr_un.sun_path, sizeof(m_pContext->addr_un.sun_path) - 1, szSocketName);
    }
#endif
    NetAddr::NetAddr(const sockaddr* pAddr, int nLen) {
        _Init();
        assert(nLen <= (int)sizeof(*m_pContext));
        memcpy(m_pContext, pAddr, nLen);
    }

    NetAddr::~NetAddr() {
        if(m_pContext) {
            free(m_pContext);
        }
    }

    sockaddr* NetAddr::GetAddr() {
        return &m_pContext->addr;
    }

    const sockaddr* NetAddr::GetAddr() const {
        return &m_pContext->addr;
    }

    int NetAddr::GetAddrLen() const {
        int nLen = 0;
        switch(m_pContext->addr.sa_family) {
            case AF_INET:
                nLen = sizeof(sockaddr_in);
                break;
#ifdef UNIX
            case AF_UNIX:
                nLen = sizeof(sockaddr_un);
                break;
#endif
            default:
                break;
        }

        return nLen;
    }

    int NetAddr::GetMaxAddrLen() const {
        return sizeof(*m_pContext);
    }

    uint32_t NetAddr::GetIP() const {
        assert(m_pContext->addr.sa_family == AF_INET);
        return m_pContext->addr_in.sin_addr.s_addr;
    }

    uint16_t NetAddr::GetPort() const {
        assert(m_pContext->addr.sa_family == AF_INET);
        return ntohs(m_pContext->addr_in.sin_port);
    }

    std::string NetAddr::IPToString() const {
        assert(m_pContext->addr.sa_family == AF_INET);
        uint8_t ucIP[4];
        char acBuffer[16];
        memcpy(ucIP, &m_pContext->addr_in.sin_addr, 4);
        sprintf(acBuffer, "%d.%d.%d.%d", (int)ucIP[0], (int)ucIP[1], (int)ucIP[2], (int)ucIP[3]);
        return acBuffer;
    }

    std::string NetAddr::ToString() const {
        switch(m_pContext->addr.sa_family) {
            case AF_INET: {
                std::string strIP = IPToString();
                char acBuffer[32];
                sprintf(acBuffer, "%s:%d", strIP.c_str(), (int)GetPort());
                return acBuffer;
				}
                break;
#ifdef UNIX
            case AF_UNIX:
                return m_pContext->addr_un.sun_path;
                break;
#endif
            default:
                assert(false);
                break;
        }

        return "";
    }

    void NetAddr::Clear() {
        memset(m_pContext, 0, sizeof(NetAddrContext_t));
        m_pContext->addr_in.sin_family = AF_INET;
    }

    const NetAddr& NetAddr::operator=(const NetAddr& rhs) {
        memcpy(m_pContext, rhs.m_pContext, sizeof(NetAddrContext_t));
		return *this;
    }

    void NetAddr::_Init() {
        m_pContext = (NetAddrContext_t*)calloc(1, sizeof(NetAddrContext_t));
        m_pContext->addr_in.sin_family = AF_INET;
    }


    NetSocket::NetSocket() : m_pNetService(NULL) {
    }

    NetSocket::~NetSocket() {
        Close();
    }

    const Socket& NetSocket::GetSocket() const {
        return m_hSocket;
    }

    bool NetSocket::BindEventHandler(NetService& nsNetService, ReadWriteHandler_t funReadEventHandler,
            ReadWriteHandler_t funWriteEventHandler) {
        UnbindEventHandler();

        if(!m_hSocket.IsEmpty()) {
            m_pNetService = &nsNetService;
            return m_pNetService->Add(m_hSocket, funReadEventHandler, funWriteEventHandler);
        } else {
            return false;
        }
    }

    void NetSocket::UnbindEventHandler() {
        if(m_pNetService && !m_hSocket.IsEmpty()) {
            m_pNetService->Delete(m_hSocket);
        }
        m_pNetService = NULL;
    }

    void NetSocket::ModifyEventHandler(ReadWriteHandler_t funReadEventHandler,
            ReadWriteHandler_t funWriteEventHandler) {
        if(m_pNetService && !m_hSocket.IsEmpty()) {
            m_pNetService->Modify(m_hSocket, funReadEventHandler, funWriteEventHandler);
        }
    }

    bool NetSocket::SetSendBufSize(int nSize) {
        return m_hSocket.SetSendBufSize(nSize);
    }

    bool NetSocket::SetRecvBufSize(int nSize) {
        return m_hSocket.SetRecvBufSize(nSize);
    }

    NetAddr NetSocket::GetLocalAddr() {
        NetAddr naAddr;
        int nAddrLen = naAddr.GetMaxAddrLen();
        m_hSocket.GetLocalAddr(naAddr.GetAddr(), nAddrLen);
        return naAddr;
    }

    NetAddr NetSocket::GetPeerAddr() {
        NetAddr naAddr;
        int nAddrLen = naAddr.GetMaxAddrLen();
        m_hSocket.GetPeerAddr(naAddr.GetAddr(), nAddrLen);
        return naAddr;
    }

    void NetSocket::Close() {
        UnbindEventHandler();
        m_hSocket.Release();
    }

    bool NetSocket::_CreateSocket(SOCKET hSocket, bool bStream, bool bAcceptor, const NetAddr* pAddr, ErrorCode& ec) {
        if(hSocket == INVALID_SOCKET) {
            hSocket = socket(AF_INET, bStream ? SOCK_STREAM : SOCK_DGRAM, 0);
        }
        if(hSocket == INVALID_SOCKET) {
            ec = ErrorCode::GetLastSystemError();
            return false;
        }
        m_hSocket.Attach(hSocket);

        if(pAddr != NULL) {
            int flag = 1;
            socklen_t len = sizeof(int);
            if(setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, len) != 0)  {
                ec = ErrorCode::GetLastSystemError();
                m_hSocket.Release();
                return false;
            }

            if(!m_hSocket.Bind(pAddr->GetAddr(), pAddr->GetAddrLen()))  {
                ec = ErrorCode::GetLastSystemError();
                m_hSocket.Release();
                return false;
            }
        }

        if(bAcceptor) {
            if(!m_hSocket.Listen(1024)) {
                ec = ErrorCode::GetLastSystemError();
                m_hSocket.Release();
                return false;
            }
        }

        if(!m_hSocket.SetNoBlockMode(true)) {
            ec = ErrorCode::GetLastSystemError();
            m_hSocket.Release();
            return false;
        }

        return true;
    }


    UDPSocket::UDPSocket(ErrorCode& ec) : NetSocket() {
        _CreateSocket(INVALID_SOCKET, false, false, NULL, ec);
    }

    UDPSocket::UDPSocket(SOCKET hSocket, ErrorCode& ec) : NetSocket() {
        _CreateSocket(hSocket, false, false, NULL, ec);
    }

    UDPSocket::UDPSocket(const NetAddr& naAddr, ErrorCode& ec) : NetSocket() {
        _CreateSocket(INVALID_SOCKET, false, false, &naAddr, ec);
    }

    UDPSocket::~UDPSocket() {
        Close();
    }

    int UDPSocket::SendTo(const uint8_t* pBuf, int nSize, const NetAddr& naAddr, ErrorCode& ec, int nTimeout) {
        int nRet = m_hSocket.TimedSendTo((const char*)pBuf, nSize, naAddr.GetAddr(), naAddr.GetAddrLen(), nTimeout);
        if(nRet > 0) {
            ec = ErrorCode();
        } else if(nRet == 0) {
            ec = AslError(AECV_OpTimeout);
        } else {
            ec = ErrorCode::GetLastSystemError();
        }

        return nRet;
    }

    int UDPSocket::RecvFrom(uint8_t* pBuf, int nSize, NetAddr& naAddr, ErrorCode& ec) {
        int nAddrLen = naAddr.GetMaxAddrLen();
        int nRet = m_hSocket.RecvFrom((char*)pBuf, nSize, naAddr.GetAddr(), nAddrLen);
        if(nRet >= 0) {
            ec = ErrorCode();
        } else {
            ec = ErrorCode::GetLastSystemError();
        }

        return nRet;
    }


    TCPSocket::TCPSocket(ErrorCode& ec) : NetSocket(), m_u64ConnTimer(0) {
        _CreateSocket(INVALID_SOCKET, true, false, NULL, ec);
    }

    TCPSocket::TCPSocket(SOCKET hSocket, ErrorCode& ec) : NetSocket(), m_u64ConnTimer(0) {
        _CreateSocket(hSocket, true, false, NULL, ec);
    }

    TCPSocket::TCPSocket(const NetAddr& naAddr, ErrorCode& ec) : NetSocket(), m_u64ConnTimer(0) {
        _CreateSocket(INVALID_SOCKET, true, false, &naAddr, ec);
    }

    TCPSocket::~TCPSocket() {
        Close();
    }

    bool TCPSocket::BindEventHandler(NetService& nsNetService, ReadWriteHandler_t funReadEventHandler,
            ReadWriteHandler_t funWriteEventHandler) {
        UnbindEventHandler();

        if(m_funConnectEventHandler) {
            if(!NetSocket::BindEventHandler(nsNetService, ReadWriteHandler_t(), [this](){
                _OnConnect(false);
            })) {
                return false;
            }
        } else {
            if(!NetSocket::BindEventHandler(nsNetService, funReadEventHandler, funWriteEventHandler)) {
                return false;
            }
        }

        m_funReadEventHandler = funReadEventHandler;
        m_funWriteEventHandler = funWriteEventHandler;

        return true;
    }

    void TCPSocket::UnbindEventHandler() {
        m_funReadEventHandler = ReadWriteHandler_t();
        m_funWriteEventHandler = ReadWriteHandler_t();
        NetSocket::UnbindEventHandler();
    }

    void TCPSocket::ModifyEventHandler(ReadWriteHandler_t funReadEventHandler,
            ReadWriteHandler_t funWriteEventHandler) {
        if(m_funConnectEventHandler) {
            NetSocket::ModifyEventHandler(ReadWriteHandler_t(), [this](){
                _OnConnect(false);
            });
        } else {
            NetSocket::ModifyEventHandler(funReadEventHandler, funWriteEventHandler);
        }

        m_funReadEventHandler = funReadEventHandler;
        m_funWriteEventHandler = funWriteEventHandler;
    }

    int TCPSocket::Connect(const NetAddr& naAddr, int nTimeout) {
        bool bRet = m_hSocket.TimedConnect(naAddr.GetAddr(), naAddr.GetAddrLen(), nTimeout);
        return bRet ? 0 : ErrorCode::GetLastSystemError();
    }

    void TCPSocket::AsyncConnect(const NetAddr& naAddr, ConnectEventHandler_t funConnectEventHandler, int nTimeout) {
        m_funConnectEventHandler = funConnectEventHandler;
        m_hSocket.Connect(naAddr.GetAddr(), naAddr.GetAddrLen());
        m_u64ConnTimer = m_pNetService->AddTimer(m_hSocket, nTimeout, [this]{_OnConnect(true);});
        ModifyEventHandler(m_funReadEventHandler, m_funWriteEventHandler);
    }

    void TCPSocket::_OnConnect(bool bTimeout) {
        if(bTimeout) {
            auto handler = m_funConnectEventHandler; // 维持上下文生命周期
            if(m_funConnectEventHandler) {
                m_funConnectEventHandler(AslError(AECV_OpTimeout));
                m_funConnectEventHandler = ConnectEventHandler_t();
            }
            ModifyEventHandler(m_funReadEventHandler, m_funWriteEventHandler);
            return;
        } else if(m_u64ConnTimer != 0) {
            m_pNetService->DeleteTimer(m_u64ConnTimer);
            m_u64ConnTimer = 0;
        }

        ErrorCode ec;
        int error;
        socklen_t len = sizeof(error);
        if(getsockopt(m_hSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0) {
            ec = ErrorCode::GetLastSystemError();
            if(!ec) {
                ec = AslError(AECV_Error);
            }
        } else if(error != 0) {
            ec = AslError(AECV_ConnectFailed);
        }

        auto funConnectEvent = m_funConnectEventHandler;
        m_funConnectEventHandler = ConnectEventHandler_t();
        if(funConnectEvent) {
            funConnectEvent(ec);
        }
        ModifyEventHandler(m_funReadEventHandler, m_funWriteEventHandler);
    }

    int TCPSocket::Send(const uint8_t* pBuf, int nSize, ErrorCode& ec, int nTimeout) {
        int nRet = m_hSocket.TimedSend((const char*)pBuf, nSize, nTimeout);
        if(nRet > 0) {
            ec = ErrorCode();
        } else if(nRet == 0) {
            ec = AslError(AECV_OpTimeout);
        } else {
            ec = ErrorCode::GetLastSystemError();
        }

        return nRet;
    }

    int TCPSocket::Recv(uint8_t* pBuf, int nSize, ErrorCode& ec) {
        int nRet = m_hSocket.Recv((char*)pBuf, nSize);
        if(nRet >= 0) {
            ec = ErrorCode();
        } else {
            ec = ErrorCode::GetLastSystemError();
        }

        return nRet;
    }


    TCPAcceptor::TCPAcceptor(const NetAddr& naAddr, ErrorCode& ec) : NetSocket() {
        _CreateSocket(INVALID_SOCKET, true, true, &naAddr, ec);
    }

    TCPAcceptor::~TCPAcceptor() {
        Close();
    }

    TCPSocket* TCPAcceptor::Accept(ErrorCode& ec) {
        SOCKET hSocket = _DoAccept(ec);
        if(hSocket == INVALID_SOCKET) {
            return NULL;
        }

        TCPSocket* pSocket = new TCPSocket(hSocket, ec);
        if(ec) {
            delete pSocket;
            return NULL;
        } else {
            return pSocket;
        }
    }

    void TCPAcceptor::Skip() {
        SOCKET hSocket = m_hSocket.Accept();
        if(hSocket != INVALID_SOCKET) {
            Socket s;
            s.Attach(hSocket);
            s.Release();
        }
    }

    SOCKET TCPAcceptor::_DoAccept(ErrorCode& ec) {
        SOCKET hSocket = m_hSocket.Accept();
        if(hSocket != INVALID_SOCKET) {
            ec = ErrorCode();
        } else {
            ec = ErrorCode::GetLastSystemError();
        }

        return hSocket;
    }
}
