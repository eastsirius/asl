/**
 * @file thread.cpp
 * @brief 多线程工具
 * @author 程行通
 */

#include "thread.hpp"
#include <cassert>
#ifdef WINDOWS
#  include <windows.h>
#  include <process.h>
#else
#  include <signal.h>
#  include <sys/prctl.h>
#  include <unistd.h>
#  include <errno.h>
#  include <sys/time.h>
#  include <pthread.h>
#  include <semaphore.h>
#endif
#include "utils_inc.hpp"
#include "time.hpp"

#ifdef ASL_DISABLE_PRCTL
#  define ASL_USE_PRCTL 0
#else
#  define ASL_USE_PRCTL 1
#endif

namespace ASL_NAMESPACE {
	struct asl_rwlock_ctx_t {
#ifdef WINDOWS
		volatile LONG count;
		volatile LONG direct;
		HANDLE finish_event;
		CRITICAL_SECTION start_lock;
#else
		pthread_rwlock_t mtx;
#endif
	};

	struct asl_semaphore_ctx_t {
#ifdef WINDOWS
		HANDLE handle;
#else
		sem_t* sem;
#endif
	};

	struct asl_thread_pool_ctx_t {
		bool bWorkerThreadRun;				///< 工作线程控制变量
		ThreadPtr_t pThread;				///< 线程类指针
		Timer tFreeTimer;					///< 释放计时器
		Semaphore semThreadSem;				///< 线程同步信号量
		ThreadPool::FuncTaskProc_t funTaskProc;	///< 任务处理函数
	};


	RWLock::RWLock()
	{
		m_hMutex = (asl_rwlock_ctx_t*)malloc(sizeof(asl_rwlock_ctx_t));
#ifdef WINDOWS
		m_hMutex->count = 0;
		m_hMutex->direct = 0;
		m_hMutex->finish_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		InitializeCriticalSection(&m_hMutex->start_lock);
#else
		pthread_rwlock_init(&m_hMutex->mtx, NULL);
#endif
	}

	RWLock::~RWLock()
	{
#ifndef WINDOWS
		pthread_rwlock_destroy(&m_hMutex->mtx);
#else
		assert(m_hMutex->count == 0);
		CloseHandle(m_hMutex->finish_event);
		DeleteCriticalSection(&m_hMutex->start_lock);
#endif
		free(m_hMutex);
	}

	void RWLock::ReadLock()
	{
#ifndef WINDOWS
		pthread_rwlock_rdlock(&m_hMutex->mtx);
#else
		EnterCriticalSection(&m_hMutex->start_lock);
		while(m_hMutex->count > 0 && m_hMutex->direct != 0)
		{
			WaitForSingleObject(m_hMutex->finish_event, INFINITE);
		}
		m_hMutex->direct = 0;
		InterlockedIncrement(&m_hMutex->count);
		LeaveCriticalSection(&m_hMutex->start_lock);
#endif
	}

	bool RWLock::TryReadLock()
	{
#ifndef WINDOWS
		return pthread_rwlock_tryrdlock(&m_hMutex->mtx) == 0;
#else
		EnterCriticalSection(&m_hMutex->start_lock);
		if(m_hMutex->count > 0 && m_hMutex->direct != 0)
		{
			LeaveCriticalSection(&m_hMutex->start_lock);
			return false;
		}
		m_hMutex->direct = 0;
		InterlockedIncrement(&m_hMutex->count);
		LeaveCriticalSection(&m_hMutex->start_lock);

		return true;
#endif
	}

	void RWLock::ReadUnlock()
	{
#ifndef WINDOWS
		pthread_rwlock_unlock(&m_hMutex->mtx);
#else
		assert(m_hMutex->count > 0);
		assert(m_hMutex->direct == 0);
		InterlockedDecrement(&m_hMutex->count);
		SetEvent(m_hMutex->finish_event);
#endif
	}

	void RWLock::WriteLock()
	{
#ifndef WINDOWS
		pthread_rwlock_wrlock(&m_hMutex->mtx);
#else
		EnterCriticalSection(&m_hMutex->start_lock);
		while(m_hMutex->count > 0 && m_hMutex->direct != 1)
		{
			WaitForSingleObject(m_hMutex->finish_event, INFINITE);
		}
		m_hMutex->direct = 1;
		InterlockedIncrement(&m_hMutex->count);
		LeaveCriticalSection(&m_hMutex->start_lock);
#endif
	}

	bool RWLock::TryWriteLock()
	{
#ifndef WINDOWS
		return pthread_rwlock_trywrlock(&m_hMutex->mtx) == 0;
#else
		EnterCriticalSection(&m_hMutex->start_lock);
		if(m_hMutex->count > 0 && m_hMutex->direct != 1)
		{
			LeaveCriticalSection(&m_hMutex->start_lock);
			return false;
		}
		m_hMutex->direct = 1;
		InterlockedIncrement(&m_hMutex->count);
		LeaveCriticalSection(&m_hMutex->start_lock);

		return true;
#endif
	}

	void RWLock::WriteUnlock()
	{
#ifndef WINDOWS
		pthread_rwlock_unlock(&m_hMutex->mtx);
#else
		assert(m_hMutex->count > 0);
		assert(m_hMutex->direct == 1);
		InterlockedDecrement(&m_hMutex->count);
		SetEvent(m_hMutex->finish_event);
#endif
	}


	Semaphore::Semaphore(int init_count)
	{
		m_hSem = (asl_semaphore_ctx_t*)malloc(sizeof(asl_semaphore_ctx_t));
#ifdef WINDOWS
		m_hSem->handle = CreateSemaphore(NULL, init_count, 0x7FFFFFFF, NULL);
		assert(m_hSem->handle != INVALID_HANDLE_VALUE);
#elif APPLE
		for(int i = 0; i < 100; ++i)
		{
			char acBuffer[32] = {0};
			for(int j = 0; j < 15; ++j)
			{
				sprintf(acBuffer + 2 * j, "%X", (int)(uint8_t)rand());
			}
			m_hSem->sem = sem_open(acBuffer, O_CREAT, 0644, init_count);
			if(m_hSem->sem != NULL)
			{
				break;
			}
		}
		assert(m_hSem->sem != NULL);
#else
		m_hSem->sem = new sem_t;
		sem_init(m_hSem->sem, 0, init_count);
#endif
	}

	Semaphore::~Semaphore()
	{
#ifdef WINDOWS
		CloseHandle(m_hSem->handle);
#elif APPLE
		sem_close(m_hSem->sem);
#else
		sem_destroy(m_hSem->sem);
		delete m_hSem->sem;
		m_hSem->sem = NULL;
#endif
		free(m_hSem);
	}

	bool Semaphore::Wait()
	{
#ifdef WINDOWS
		return WaitForSingleObject(m_hSem->handle, -1) == WAIT_OBJECT_0;
#else
		return sem_wait(m_hSem->sem) == 0;
#endif
	}

	bool Semaphore::TryWait()
	{
#ifdef WINDOWS
		return WaitForSingleObject(m_hSem->handle, 0) == WAIT_OBJECT_0;
#else
		return sem_trywait(m_hSem->sem) == 0;
#endif
	}

	bool Semaphore::TimedWait(int timeout)
	{
		if(timeout < 0)
		{
			return Wait();
		}

#ifdef WINDOWS
		return WaitForSingleObject(m_hSem->handle, timeout) == WAIT_OBJECT_0;
#elif APPLE
		assert(false);
		return false;
#else
		timespec ts = { timeout / 1000, timeout % 1000 * 1000000 };
		return sem_timedwait(m_hSem->sem, &ts) == 0;
#endif
	}

	bool Semaphore::Post()
	{
#ifdef WINDOWS
		return ReleaseSemaphore(m_hSem->handle, 1, NULL) != FALSE;
#else
		return sem_post(m_hSem->sem) == 0;
#endif
	}


	Thread::Thread() {
	}

	Thread::Thread(std::function<void()> proc, const std::string name) 
		: m_strName(name), m_hThread([proc, name](){
#ifdef LINUX
#  if ASL_USE_PRCTL
			if(name != "") {
				char buf[20] = {0};
				strncpy(buf, name.c_str(), sizeof(buf) - 1);
				prctl(PR_SET_NAME, (unsigned long)buf);
			}
#  endif
#endif
			proc();
		}) {
	}

	void Thread::Sleep(int timeout) {
#ifdef WINDOWS
		::Sleep(timeout);
#else
		struct timespec ts = { timeout / 1000, timeout % 1000 * 1000 * 1000};
		nanosleep(&ts, NULL);
#endif
	}

	
	ThreadGroup::ThreadGroup() {
	}

	ThreadGroup::~ThreadGroup() {
		AutoLocker<Mutex> locker(m_mtxThreadsLock);
		m_lstThreads.clear();
	}

	ThreadPtr_t ThreadGroup::CreateThread(std::function<void()> proc, const std::string name) {
		auto pThread = std::make_shared<Thread>(proc, name);
		AddThread(pThread);

		return pThread;
	}

	void ThreadGroup::AddThread(ThreadPtr_t thread) {
		if(thread) {
			AutoLocker<Mutex> locker(m_mtxThreadsLock);
			m_lstThreads.push_back(thread);
		}
	}

	void ThreadGroup::RemoveThread(ThreadPtr_t thread) {
		AutoLocker<Mutex> locker(m_mtxThreadsLock);
		for(size_t i = 0; i < m_lstThreads.size(); ++i) {
			if(thread == m_lstThreads[i]) {
				m_lstThreads.erase(m_lstThreads.begin() + i);
				break;
			}
		}
	}

	void ThreadGroup::DeleteThread(ThreadPtr_t thread) {
		ThreadPtr_t pTemp;
		{
			AutoLocker<Mutex> locker(m_mtxThreadsLock);
			for(size_t i = 0; i < m_lstThreads.size(); ++i) {
				if(m_lstThreads[i] == thread) {
					pTemp = m_lstThreads[i];
					m_lstThreads.erase(m_lstThreads.begin() + i);
					break;
				}
			}
		}

		if(pTemp) {
			if(pTemp->Joinable()) {
				pTemp->Join();
			}
		}
	}

	void ThreadGroup::DeleteAll() {
		std::vector<ThreadPtr_t> lstThreads;
		{
			AutoLocker<Mutex> locker(m_mtxThreadsLock);
			lstThreads = m_lstThreads;
			m_lstThreads.clear();
		}

		for(size_t i = 0; i < lstThreads.size(); ++i) {
			if(lstThreads[i]->Joinable()) {
				lstThreads[i]->Join();
			}
		}
	}

	bool ThreadGroup::IsThreadIn(ThreadPtr_t thread) {
		AutoLocker<Mutex> locker(m_mtxThreadsLock);
		for(size_t i = 0; i < m_lstThreads.size(); ++i) {
			if(thread == m_lstThreads[i]) {
				return true;
			}
		}

		return false;
	}


	ThreadPool::ThreadPool() : m_nMaxThreadNum(0), m_bWorking(false), m_pFreeThreadSem(NULL) {
	}

	ThreadPool::~ThreadPool() {
	}

	bool ThreadPool::Create(int max_thread_num) {
		m_nMaxThreadNum = max_thread_num;

		m_pFreeThreadSem = new Semaphore(m_nMaxThreadNum);
		ThreadContext_t* pContext = _AllocThreadContext();
		if(!pContext) {
			Release();
			return false;
		}
		m_bWorking = true;
		_FreeThreadContext(pContext);

		return true;
	}

	void ThreadPool::Release() {
		m_mtxFreeThreadListLock.Lock();
		m_bWorking = false;
		for(size_t i = 0; i < m_lstFreeThreadList.size(); ++i) {
			m_lstFreeThreadList[i]->bWorkerThreadRun = false;
			m_lstFreeThreadList[i]->semThreadSem.Post();
		}
		m_mtxFreeThreadListLock.Unlock();
		m_tgThreadGroup.DeleteAll();

		m_mtxFreeThreadListLock.Lock();
		while(!m_lstFreeThreadList.empty()) {
			delete m_lstFreeThreadList.front();
			m_lstFreeThreadList.pop_front();
		}
		m_mtxFreeThreadListLock.Unlock();

		if(m_pFreeThreadSem != NULL) {
			delete m_pFreeThreadSem;
			m_pFreeThreadSem = NULL;
		}
	}

	bool ThreadPool::Exec(FuncTaskProc_t proc, int timeout) {
		if(!m_bWorking) {
			return false;
		}

		if(!m_pFreeThreadSem->TimedWait(timeout)) {
			return false;
		}

		ThreadContext_t* pContext = _AllocThreadContext();
		if(pContext == NULL) {
			m_pFreeThreadSem->Post();
			return false;
		}

		pContext->funTaskProc = proc;
		pContext->semThreadSem.Post();

		return true;
	}

	void ThreadPool::_WorkerThreadProc(ThreadContext_t* context) {
		assert(context != NULL);

		while(context->bWorkerThreadRun) {
			context->semThreadSem.Wait();
			if(context->bWorkerThreadRun) {
				context->funTaskProc();
				_FreeThreadContext(context);
				m_pFreeThreadSem->Post();
			}
		}
	}

	ThreadPool::ThreadContext_t* ThreadPool::_AllocThreadContext() {
		ThreadContext_t* pContext = NULL;
		m_mtxFreeThreadListLock.Lock();
		if(!m_lstFreeThreadList.empty()) {
			pContext = m_lstFreeThreadList.front();
			m_lstFreeThreadList.pop_front();
		}
		_ReleaseFreeThreads();
		m_mtxFreeThreadListLock.Unlock();

		if(pContext == NULL) {
			pContext = new ThreadContext_t();
			pContext->bWorkerThreadRun = true;
			pContext->pThread = m_tgThreadGroup.CreateThread(std::bind(&ThreadPool::_WorkerThreadProc, this, pContext));
			if(pContext->pThread == NULL) {
				delete pContext;
				return NULL;
			}
		}

		return pContext;
	}

	void ThreadPool::_FreeThreadContext(ThreadContext_t* context) {
		assert(context != NULL);

		m_mtxFreeThreadListLock.Lock();
		_ReleaseFreeThreads();
		if(!m_bWorking) {
			context->bWorkerThreadRun = false;
			context->semThreadSem.Post();
		}
		context->tFreeTimer.Restart();
		m_lstFreeThreadList.push_front(context);
		m_mtxFreeThreadListLock.Unlock();
	}

	void ThreadPool::_ReleaseFreeThreads() {
		while(!m_lstFreeThreadList.empty()) {
			ThreadContext_t* pContext = m_lstFreeThreadList.back();
			if(pContext->tFreeTimer.MillisecTime() > 30000) {
				pContext->bWorkerThreadRun = false;
				pContext->semThreadSem.Post();
				m_tgThreadGroup.DeleteThread(pContext->pThread);
				m_lstFreeThreadList.pop_back();
			} else {
				break;
			}
		}
	}
}
