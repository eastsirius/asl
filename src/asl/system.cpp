/**
 * @file system.cpp
 * @brief 系统工具
 * @author 程行通
 */

#include "system.hpp"
#include <stdlib.h>
#include <cassert>
#include <cstring>
#ifndef WINDOWS
#  include <unistd.h>
#  include <net/if.h>
#  include <sys/ioctl.h>
#  include <errno.h>
#  include <dlfcn.h>
#else
#  include <windows.h>
#  include <iphlpapi.h>
#endif

namespace ASL_NAMESPACE {
	CustomErrorDesc::CustomErrorDesc(const std::string& msg) : m_strMessage(msg) {
	}

	CustomErrorDesc::~CustomErrorDesc() {
	}
	
	const char* CustomErrorDesc::GetMsg(int ec) const {
		return m_strMessage.c_str();
	}


	SystemErrorDesc::SystemErrorDesc() {
	}

	SystemErrorDesc::~SystemErrorDesc() {
	}

	const char* SystemErrorDesc::GetMsg(int ec) const {
#ifndef WINDOWS
		return strerror(ec);
#else
		int iStringLength;
		__declspec(thread) static char szErrorString[1024];//存放信息字符串的线程局部内存
		__declspec(thread) static HMODULE hNetmsgModule = NULL;
		__declspec(thread) static HMODULE hWininetModule = NULL;

		if((ec >= 2100) && (ec <= 2999)) {
			//如果是lmerr.h、netmsg.dll模块包含的错误码
			if(hNetmsgModule == NULL) {
				hNetmsgModule = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
				if(hNetmsgModule == NULL) {
					return "";
				}
			}

			iStringLength = FormatMessage (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hNetmsgModule, ec, 0, szErrorString, sizeof(szErrorString), NULL);
		} else if((ec >= 12000) && (ec <= 12175)) {
			//如果是wininet.h、wininet.dll模块包含的错误码
			if(hWininetModule == NULL) {
				hWininetModule = LoadLibraryEx(TEXT("wininet.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
				if(hWininetModule == NULL) {
					return "";
				}
			}

			iStringLength = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hWininetModule, ec, 0, szErrorString, sizeof(szErrorString), NULL);
		} else {
			//如果是其他系统的错误码
			iStringLength = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ec, 0, szErrorString, sizeof(szErrorString), NULL);
		}

		if(iStringLength > 0) {
			//如果成功获取到错误信息字符串，去掉字符串末尾的“\r\n”
			if(szErrorString[iStringLength - 2] == '\r') {
				if(szErrorString[iStringLength - 1] == '\n') {
					iStringLength -= 2;
				} else {
					iStringLength--;
				}

				szErrorString[iStringLength] = '\0';
			}
			return szErrorString;
		} else {
			return "unknown system error";
		}
#endif
	}


	AslErrorDesc::AslErrorDesc() {
	}

	AslErrorDesc::~AslErrorDesc() {
	}

	const char* AslErrorDesc::GetMsg(int ec) const {
		switch(ec) {
		case AECV_Error:
			return "error";
		case AECV_AllocContextError:
			return "error when alloc context";
		case AECV_OpTimeout:
			return "operate timeout";
		case AECV_Reconnect:
			return "reconnect";
		case AECV_ConnectFailed:
			return "connect failed";
		case AECV_BindSocketError:
			return "error when bind socket to netservice";
		case AECV_ParamError:
			return "param error";
		case AECV_AllocMemoryFailed:
			return "alloc memory failed";
		case AECV_SerialFailed:
		    return "serial data failed";
        case AECV_ParseFailed:
            return "parse data failed";
        case AECV_SendFailed:
            return "send data failed";
        case AECV_RecvFailed:
            return "recv data failed";
		default:
			break;
		}

		return "unknown asl error";
	}


	ErrorCode::ErrorCode() : m_nCode(0) {
	}

	ErrorCode::ErrorCode(const ErrorCode& ec) : m_nCode(ec.m_nCode), m_pDesc(ec.m_pDesc) {
	}

	ErrorCode::ErrorCode(int ec, ErrorDesc* desc) : m_nCode(ec), m_pDesc(desc) {
	}

	const char* ErrorCode::GetType() const {
		if(m_pDesc) {
			return m_pDesc->GetType();
		} else {
			return "unknown type";
		}
	}

	int ErrorCode::GetCode() const {
		return m_nCode;
	}

	const char* ErrorCode::GetMsg() const {
		if(m_pDesc) {
			return m_pDesc->GetMsg(m_nCode);
		} else if(m_nCode == 0) {
			return "no error";
		} else {
			return "unknown error";
		}
	}

	ErrorCode ErrorCode::GetLastSystemError() {
#ifdef WINDOWS
		return SystemError(::GetLastError());
#else
		return SystemError(errno);
#endif
	}


	Error::Error(ErrorCode ec) : m_ecErrorCode(ec) {
	}

	Error::~Error() throw() {
	}

	const char* Error::what() const throw() {
		return m_ecErrorCode.GetMsg();
	}

	const ErrorCode& Error::GetCode() const throw() {
		return m_ecErrorCode;
	}


	int System::GetNumberOfProcessors() {
#ifdef WINDOWS
		SYSTEM_INFO info = {0};
		GetSystemInfo(&info);
		return info.dwNumberOfProcessors;
#else
		return (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
	}


	struct asl_dlloader_ctx_t {
#ifdef WINDOWS
		HMODULE ctx;
#else
		void* ctx;
#endif
	};


	DlLoader::DlLoader() {
		m_hHandle = (asl_dlloader_ctx_t*)malloc(sizeof(asl_dlloader_ctx_t));
		m_hHandle->ctx = NULL;
	}

	DlLoader::~DlLoader() {
		Release();
		free(m_hHandle);
	}

	bool DlLoader::Load(const char* filename) {
		assert(filename != NULL);
		Release();

#ifdef WINDOWS
		m_hHandle->ctx = LoadLibraryA(filename);
#else
		m_hHandle->ctx = dlopen(filename, 0);
#endif

		return m_hHandle->ctx != NULL;
	}

	void DlLoader::Release() {
		if(m_hHandle->ctx != NULL) {
#ifdef WINDOWS
			FreeLibrary(m_hHandle->ctx);
#else
			dlclose(m_hHandle->ctx);
#endif
			m_hHandle->ctx = NULL;
		}
	}

	void* DlLoader::GetProc(const char* proc_name) {
		assert(proc_name != NULL);
		if(m_hHandle->ctx == NULL) {
			return NULL;
		}

#ifdef WINDOWS
		return GetProcAddress(m_hHandle->ctx, proc_name);
#else
		return dlsym(m_hHandle->ctx, proc_name);
#endif
	}


	DlProcList::DlProcList() {
	}

	DlProcList::~DlProcList() {
		Release();
	}

	bool DlProcList::Load(const char* filename) {
		Release();

		return DlLoader::Load(filename);
	}

	bool DlProcList::Load(const char *filename, const char** proc_names, int proc_names_num) {
		assert(filename != NULL);
		assert(proc_names != NULL);

		Release();

		if(!DlLoader::Load(filename)) {
			return false;
		}

		for(int i = 0; i < proc_names_num; ++i) {
			if(AddProc(proc_names[i]) == -1) {
				Release();
				return false;
			}
		}

		return true;
	}

	void DlProcList::Release() {
		m_lstDllProcs.clear();
		DlLoader::Release();
	}

	int DlProcList::AddProc(const char *proc_name) {
		assert(proc_name != NULL);

		void *pProc = GetProc(proc_name);
		if(pProc == NULL) {
			return -1;
		}

		m_lstDllProcs.push_back(pProc);

		return (int)m_lstDllProcs.size() - 1;
	}

	void* DlProcList::operator[](size_t index) const {
		assert(index < m_lstDllProcs.size());
		return m_lstDllProcs[index];
	}
}
