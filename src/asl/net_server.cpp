/**
 * @file net_server.cpp
 * @brief 网络服务
 * @author 程行通
 */

#include "net_server.hpp"
#include <cstring>

namespace ASL_NAMESPACE {
    NetServer::NetServer() {
    }

    NetServer::~NetServer() {
    }


    BaseTCPServer::BaseTCPServer(NetService& nsNetService)
            : NetServer(), m_nsNetService(nsNetService) {
        m_nConnIdCount = rand();
    }

    BaseTCPServer::~BaseTCPServer() {
    }

    bool BaseTCPServer::Start(const NetServerParam* pParam) {
        const BaseTCPServerParam* param = dynamic_cast<const BaseTCPServerParam*>(pParam);
        if(!param) {
            return false;
        }

        if(!_CreateListeners(param)) {
            return false;
        }

        return true;
    }

    void BaseTCPServer::Stop() {
        _ReleaseListeners();
    }

    bool BaseTCPServer::_SendData(int64_t nConnId, const uint8_t* pData, int nSize, int nTimeout) {
        auto pSession = _GetSession(nConnId, false);
        if(!pSession) {
            return false;
        }

        AutoLocker<Mutex> locker(pSession->mtxLock);
        return pSession->pSocket->Send(pData, nSize, nTimeout) == nSize;
    }

    bool BaseTCPServer::_CreateListeners(const BaseTCPServerParam* pParam) {
        AutoLocker<Mutex> alLock(m_mtxListenersLock);
        assert(m_lstListeners.empty());

        for(size_t i = 0; i < pParam->lstListeners.size(); ++i) {
            std::string strAddr = pParam->lstListeners[i];
            std::string strIP = "0.0.0.0";
            int nPort = 0;
            if(strAddr.find(":") == std::string::npos) {
                nPort = atoi(strAddr.c_str());
            } else {
                int nPos = strAddr.find(":");
                strIP = std::string(strAddr.begin(), strAddr.begin() + nPos);
                std::string strPort(strAddr.begin() + nPos + 1, strAddr.end());
                nPort = atoi(strPort.c_str());
            }

            if(nPort <= 0 ){
                return false;
            }

            ErrorCode ec;
            std::shared_ptr<TCPAcceptor> pSocket = std::make_shared<TCPAcceptor>(NetAddr(strIP.c_str(), nPort), ec);
            if(ec) {
                return false;
            }
            auto handler = [this, pSocket]() {
                this->_OnListenerRead(pSocket.get());
            };
            if(!pSocket->BindEventHandler(m_nsNetService, handler)) {
                pSocket->Close();
                return false;
            }
            m_lstListeners.push_back(pSocket);
        }

        return true;
    }

    void BaseTCPServer::_ReleaseListeners() {
        AutoLocker<Mutex> alLock(m_mtxListenersLock);

        for(size_t i = 0; i < m_lstListeners.size(); ++i) {
            m_lstListeners[i]->Close();
        }
        m_lstListeners.clear();
    }

    void BaseTCPServer::_Disconnect(int64_t nConnId) {
        auto pSession = _GetSession(nConnId, true);
        if(pSession) {
            AutoLocker<Mutex> locker(pSession->mtxLock);
            pSession->pSocket->Close();
        }
    }

    void BaseTCPServer::_OnListenerRead(TCPAcceptor* pAcceptor) {
        TCPConnSessionPtr_t pSession = std::make_shared<TCPConnSession>();
        if(!pSession->bfRecvBuffer.RequestFreeSize(64 * 1024)) {
            return;
        }

        auto pSocket = pAcceptor->Accept();
        if(!pSocket) {
            //接收失败，可能是惊群现象引起
            return;
        }

        int64_t nConnId = ++m_nConnIdCount;
        auto readHandler = [this, nConnId](){
            _OnRead(nConnId);
        };
        if(!pSocket->BindEventHandler(m_nsNetService, readHandler)) {
            pSocket->Close();
            delete pSocket;
            return;
        }

        pSession->pSocket.reset(pSocket);
        m_mpConnections[nConnId] = pSession;
    }

    void BaseTCPServer::_OnRead(int64_t nConnId) {
        auto pSession = _GetSession(nConnId, false);
        if(!pSession) {
            return;
        }

        auto pBuf = &pSession->bfRecvBuffer;
        pBuf->RequestFreeSize(32 * 1024);
        pSession->mtxLock.Lock();
        int nRet = pSession->pSocket->Recv(pBuf->GetBuffer(pBuf->GetDataSize()), pBuf->GetFreeSize());
        pSession->mtxLock.Unlock();
        if(nRet <= 0) {
            _Disconnect(nConnId);
            return;
        }
        pBuf->AppendData(nRet);

        size_t nParsed = 0;
        while(nParsed < pBuf->GetDataSize()) {
            nRet = _ParseData(nConnId, pBuf->GetBuffer(nParsed), pBuf->GetDataSize() - nParsed);
            if(nRet < 0) {
                //服务主动断开
                _Disconnect(nConnId);
                return;
            } else if(nRet == 0) {
                //数据不足
                break;
            }
            nParsed += nRet;
        }
        assert(nParsed <= pBuf->GetDataSize());
        pBuf->SkipData(nParsed);
    }

    BaseTCPServer::TCPConnSessionPtr_t BaseTCPServer::_GetSession(int64_t nConnId, bool bDelete) {
        AutoLocker<Mutex> locker(m_mtxConnectionsLock);
        auto iter = m_mpConnections.find(nConnId);
        if(iter == m_mpConnections.end()) {
            return NULL;
        }

        if(bDelete) {
            auto ret = iter->second;
            m_mpConnections.erase(iter);
            return ret;
        } else {
            return iter->second;
        }
    }


    TcpRpcClient::TcpRpcClient() {
    }

    TcpRpcClient::~TcpRpcClient() {
        Close();
    }

    void TcpRpcClient::Close() {
        m_bfSendBuf.Release();
        m_bfRecvBuf.Release();
        m_funHandler = ResponseHandler_t();
        if(m_pSocket) {
            m_pSocket->Close();
            m_pSocket.reset();
        }
    }

    TcpRpcClientPtr_t TcpRpcClient::AsyncCall(NetService& nsNetService, const NetAddr& naAddr,
            const uint8_t* pData, int nSize, int nTimeout, ResponseHandler_t funHandler) {
        auto pClient = std::make_shared<TcpRpcClient>();
        if(!pClient->_AsyncCall(nsNetService, naAddr, pData, nSize, nTimeout, funHandler)) {
            pClient->Close();
            return NULL;
        }

        return pClient;
    }

    bool TcpRpcClient::_AsyncCall(NetService& nsNetService, const NetAddr& naAddr, const uint8_t* pData,
            int nSize, int nTimeout, ResponseHandler_t funHandler) {
        m_funHandler = funHandler;
        if(!m_bfSendBuf.AppendData(pData, nSize)) {
            return false;
        }

        ErrorCode ec;
        m_pSocket = std::make_shared<TCPSocket>(ec);
        if(ec) {
            _DoError(ec);
            return false;
        }
        if(!m_pSocket->BindEventHandler(nsNetService, [this]{_OnRead();})) {
            _DoError(AslError(AECV_BindSocketError));
            return false;
        }
        m_pSocket->AsyncConnect(naAddr, [this](ErrorCode ec){
            _OnConnect(ec);
        }, nTimeout);

        return true;
    }

    void TcpRpcClient::_OnConnect(ErrorCode ec) {
        if(ec) {
            _DoError(ec);
        } else {
            _DoSend();
        }
    }

    void TcpRpcClient::_OnRead() {
        if(!m_bfRecvBuf.RequestFreeSize(64 * 1024)) {
            _DoError(AslError(AECV_AllocMemoryFailed));
            return;
        }

        ErrorCode ec;
        int ret = m_pSocket->Recv(m_bfRecvBuf.GetBuffer(m_bfRecvBuf.GetDataSize()),
                m_bfRecvBuf.GetFreeSize(), ec);
        if(ec) {
            _DoError(ec);
            return;
        }
        m_bfRecvBuf.AppendData(ret);

        if(m_funHandler(m_bfRecvBuf.GetBuffer(), m_bfRecvBuf.GetDataSize(), ErrorCode())) {
            Close();
            return;
        }
    }

    void TcpRpcClient::_OnWrite() {
        _DoSend();
    }

    void TcpRpcClient::_DoError(ErrorCode ec) {
        m_funHandler(NULL, 0, ec);
        Close();
    }

    void TcpRpcClient::_DoSend() {
        if(!m_pSocket) {
            return;
        }

        ErrorCode ec;
        int ret = m_pSocket->Send(m_bfSendBuf.GetBuffer(), m_bfSendBuf.GetDataSize(), ec, 0);
        if(ec) {
            _DoError(ec);
            return;
        }
        m_bfSendBuf.SkipData(ret);

        if(m_bfSendBuf.GetDataSize() > 0) {
            m_pSocket->ModifyEventHandler([this](){_OnRead();}, [this](){_OnWrite();});
        } else {
            m_pSocket->ModifyEventHandler([this](){_OnRead();});
        }
    }
}
