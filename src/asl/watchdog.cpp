/**
 * @file watchdog.cpp
 * @brief 看门狗
 * @author 程行通
 */

#include "watchdog.hpp"
#include "thread.hpp"
#include "time.hpp"
#include <map>

namespace ASL_NAMESPACE {
	struct WdtCtx {
		WatchDog* handle;
		int timeout;
		Timer tr;
		std::function<void()> handler;
	};

	static Mutex g_mtxWdtLock;
	static std::map<WatchDog*, std::shared_ptr<WdtCtx> > g_mpWdtList;

	static int _do_init_watchdog() {
		auto worker_thread = std::make_shared<Thread>([]() {
			while(true) {
				Thread::Sleep(10);

				std::vector<std::function<void()> > handlers;

				g_mtxWdtLock.Lock();
				for(auto iter = g_mpWdtList.begin(); iter != g_mpWdtList.end(); ) {
					if(iter->second->tr.MillisecTime() > iter->second->timeout) {
						handlers.push_back(iter->second->handler);
						iter = g_mpWdtList.erase(iter);
					} else {
						++iter;
					}
				}
				g_mtxWdtLock.Unlock();

				for(auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
					(*iter)();
				}
			}
		});
		worker_thread->Detach();

		return 0;
	}

	ASL_INIT_ONCE_PROC(init_watchdog(), _do_init_watchdog(), Thread::Sleep(1))

	
	WatchDog::WatchDog() {
	}

	WatchDog::~WatchDog() {
		Stop();
	}

	void WatchDog::Start(int nMsTimeout, std::function<void()> funHandler) {
		init_watchdog();

		AutoLocker<Mutex> locker(g_mtxWdtLock);

		auto ctx = std::make_shared<WdtCtx>();
		ctx->handle = this;
		ctx->timeout = nMsTimeout;
		ctx->handler = funHandler;
		g_mpWdtList[this] = ctx;
	}

	void WatchDog::Stop() {
		std::function<void()> handler;
		{
			AutoLocker<Mutex> locker(g_mtxWdtLock);

			auto iter = g_mpWdtList.find(this);
			if(iter != g_mpWdtList.end()) {
				handler = iter->second->handler;
				g_mpWdtList.erase(this);
			}
		}
	}

	void WatchDog::Reset() {
		AutoLocker<Mutex> locker(g_mtxWdtLock);

		auto iter = g_mpWdtList.find(this);
		if(iter != g_mpWdtList.end()) {
			iter->second->tr.Restart();
		}
	}
}
