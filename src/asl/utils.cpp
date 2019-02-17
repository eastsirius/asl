/**
 * @file utils.cpp
 * @brief 实用工具集
 * @author 程行通
 */

#include "utils.hpp"
#ifdef WINDOWS
#  include <WinSock2.h>
#  include <Windows.h>
#else
#  include <unistd.h>
#  include <signal.h>
#  include <fcntl.h>
#endif
#include <assert.h>

namespace ASL_NAMESPACE {
	NoCreatable::NoCreatable() {
	}


	NoCopyable::NoCopyable() {
	}

	NoCopyable::NoCopyable(const NoCopyable& src) {
	}

	const NoCopyable& NoCopyable::operator=(const NoCopyable& src) {
		return *this;
	}


#ifdef WINDOWS
	static BOOL WINAPI _asl_app_signal_manager_console_ctrl_header(DWORD dwCtrlType) {
		switch(dwCtrlType) {
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			if(SignalManager::Instance().OnKill()) {
				Sleep(60000);
			}
			return TRUE;
			break;

		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
			if(SignalManager::Instance().OnStop()) {
				Sleep(60000);
			}
			return TRUE;
			break;

		case CTRL_LOGOFF_EVENT:
			return FALSE;
			break;

		default:
			return FALSE;
		}
	}
#else
	static void _asl_app_signal_manager_on_stop(int sig) {
		SignalManager::Instance().m_funOnStop();
	}

	static void _asl_app_signal_manager_on_kill(int sig) {
		SignalManager::Instance().m_funOnKill();
	}
#endif

	bool SignalManager::RegisterSignals(SignalProc_t stop_proc, SignalProc_t kill_proc) {
		m_funOnStop = stop_proc;
		m_funOnKill = kill_proc;
#ifdef WINDOWS
		return SetConsoleCtrlHandler(_asl_app_signal_manager_console_ctrl_header, TRUE) != NULL;
#else
		if(stop_proc) {
			signal(SIGINT, _asl_app_signal_manager_on_stop);
			signal(SIGQUIT, _asl_app_signal_manager_on_stop);
			signal(SIGSTOP, _asl_app_signal_manager_on_stop);
			signal(SIGTSTP, _asl_app_signal_manager_on_stop);
			signal(SIGTERM, _asl_app_signal_manager_on_stop);
		}
		if(kill_proc) {
			signal(SIGKILL, _asl_app_signal_manager_on_kill);
		}

		return true;
#endif
	}

	void SignalManager::UnregisterSignals() {
#ifdef WINDOWS
		SetConsoleCtrlHandler(_asl_app_signal_manager_console_ctrl_header, FALSE);
#else
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGSTOP, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGKILL, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
#endif
	}

	SignalManager& SignalManager::Instance() {
		static SignalManager instance;
		return instance;
	}

	bool SignalManager::OnStop() {
		if(m_funOnStop) {
			m_funOnStop();
			return true;
		} else {
			return false;
		}
	}

	bool SignalManager::OnKill() {
		if(m_funOnKill) {
			m_funOnKill();
			return true;
		} else {
			return false;
		}
	}
}
