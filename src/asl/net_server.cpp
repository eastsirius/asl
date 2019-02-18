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

    bool BaseTCPServer::_SendData(TCPSocket* pSocket, const uint8_t* pData, int nSize, int nTimeout) {
        m_mtxConnectionsLock.Lock();
        bool bRet = m_mpConnections.find(pSocket) != m_mpConnections.end();
        m_mtxConnectionsLock.Unlock();
        if(!bRet) {
            return false;
        }
        return pSocket->Send(pData, nSize, nTimeout) == nSize;
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
                std::string strPort(strAddr.begin() + 1, strAddr.end());
                nPort = atoi(strPort.c_str());
            }

            if(nPort <= 0 ){
                return false;
            }

            ErrorCode ec;
            auto handler = [this](NetSocket* pSocket) {
                this->_OnListenerRead((TCPAcceptor*)pSocket);
            };
            std::shared_ptr<TCPAcceptor> pSocket = std::make_shared<TCPAcceptor>(m_nsNetService,
                    NetAddr(strAddr.c_str(), nPort), ec, handler);
            if(ec) {
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

    void BaseTCPServer::_Disconnect(TCPSocket* pSocket) {
        m_mtxConnectionsLock.Lock();
        m_mpConnections.erase(pSocket);
        m_mtxConnectionsLock.Unlock();

        pSocket->Close();
        delete pSocket;
    }

    void BaseTCPServer::_OnListenerRead(TCPAcceptor* pAcceptor) {
        TCPConnSessionPtr_t pSession = std::make_shared<TCPConnSession>();
        if(!pSession->bfRecvBuffer.RequestFreeSize(64 * 1024)) {
            return;
        }

        auto readHandler = [this, pSession](NetSocket* pSocket){
            this->_OnRead((TCPSocket*)pSocket, pSession.get());
        };
        auto pSocket = pAcceptor->Accept(m_nsNetService, readHandler);
        if(!pSocket) {
            //接收失败，可能是惊群现象引起
            return;
        }

        m_mpConnections[pSocket] = pSession;
    }

    void BaseTCPServer::_OnRead(TCPSocket* pSocket, TCPConnSession* pSession) {
        auto pBuf = &pSession->bfRecvBuffer;
        pBuf->RequestFreeSize(32 * 1024);
        int nRet = pSocket->Recv(pBuf->GetBuffer(pBuf->GetDataSize()), pBuf->GetFreeSize());
        if(nRet <= 0) {
            _Disconnect(pSocket);
            return;
        }
        pBuf->AppendData(nRet);

        size_t nParsed = 0;
        while(nParsed < pBuf->GetDataSize()) {
            nRet = _ParseData(pSocket, pBuf->GetBuffer(nParsed), pBuf->GetDataSize() - nParsed);
            if(nRet < 0) {
                //服务主动断开
                _Disconnect(pSocket);
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
}
