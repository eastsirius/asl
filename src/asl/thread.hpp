/**
 * @file thread.hpp
 * @brief 多线程工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace ASL_NAMESPACE {
	struct asl_rwlock_ctx_t;
	struct asl_semaphore_ctx_t;
	struct asl_thread_pool_ctx_t;

	namespace helper {
		class null_mutex {
		public:
			null_mutex() {
			}
			void lock() {
			}
			bool try_lock() {
				return true;
			}
			void unlock() {
			}
		};
	}

	/**
	 * @brief 临界区模板
	 */
	template <class T>
	class MutexImpl : public NoCopyable {
	public:
		MutexImpl() {
		}

		typedef T BaseType;

	public:
		/**
		 * @brief 锁定临界区
		 */
		void Lock() {
			m_hMutex.lock();
		}

		/**
		 * @brief 尝试锁定临界区
		 * @return 返回执行结果
		 */
		bool TryLock() {
			return m_hMutex.try_lock();
		}

		/**
		 * @brief 解锁临界区
		 */
		void Unlock() {
			m_hMutex.unlock();
		}

		/**
		 * @brief 获取上下文
		 * @return 返回上下文
		 */
		T& GetHandle() {
			return m_hMutex;
		}
		const T& GetHandle() const {
			return m_hMutex;
		}

	private:
		T m_hMutex; ///< 临界区
	};

	typedef MutexImpl<std::mutex> Mutex; ///< 临界区
	typedef MutexImpl<helper::null_mutex> NullMutex; ///< 空临界区

	/**
	 * @brief 自动锁
	 */
	template <class T>
	class AutoLocker : public NoCopyable, public std::unique_lock<typename T::BaseType> {
	public:
		AutoLocker(T& lock) : std::unique_lock<typename T::BaseType>(lock.GetHandle()) {
		}
	};

	/**
	 * @brief 读写锁
	 */
	class RWLock : public NoCopyable {
	public:
		RWLock();
		~RWLock();

	public:
		/**
		 * @brief 读锁定
		 */
		void ReadLock();

		/**
		 * @brief 尝试读锁定
		 * @return 返回执行结果
		 */
		bool TryReadLock();

		/**
		 * @brief 读解锁
		 */
		void ReadUnlock();

		/**
		 * @brief 写锁定
		 */
		void WriteLock();

		/**
		 * @brief 尝试写锁定
		 * @return 返回执行结果
		 */
		bool TryWriteLock();

		/**
		 * @brief 写解锁
		 */
		void WriteUnlock();

	private:
		asl_rwlock_ctx_t* m_hMutex; ///< 临界区句柄
	};

	/**
	 * @brief 空读写锁
	 */
	class NullRWLock : public NoCopyable {
	public:
		NullRWLock() {
		}
		~NullRWLock() {
		}

	public:
		/**
		 * @brief 读锁定
		 */
		void ReadLock() {
		}

		/**
		 * @brief 尝试读锁定
		 * @return 返回执行结果
		 */
		bool TryReadLock() {
			return true;
		}

		/**
		 * @brief 读解锁
		 */
		void ReadUnlock() {
		}

		/**
		 * @brief 写锁定
		 */
		void WriteLock() {
		}

		/**
		 * @brief 尝试写锁定
		 * @return 返回执行结果
		 */
		bool TryWriteLock() {
			return true;
		}

		/**
		 * @brief 写解锁
		 */
		void WriteUnlock() {
		}
	};

	/**
	 * @brief 读自动锁
	 */
	template <class MutexType>
	class AutoReadLocker : public NoCopyable {
	public:
		AutoReadLocker(MutexType& lock) : m_mtxLock(lock) {
			m_mtxLock.ReadLock();
		}
		~AutoReadLocker() {
			m_mtxLock.ReadUnlock();
		}

	private:
		MutexType& m_mtxLock; ///< 临界区引用
	};

	/**
	 * @brief 写自动锁
	 */
	template <class MutexType>
	class AutoWriteLocker : public NoCopyable {
	public:
		AutoWriteLocker(MutexType& lock) : m_mtxLock(lock) {
			m_mtxLock.WriteLock();
		}
		~AutoWriteLocker() {
			m_mtxLock.WriteUnlock();
		}

	private:
		MutexType& m_mtxLock; ///< 临界区引用
	};

	/**
	 * @brief 条件变量
	 */
	class Conditional : public NoCopyable {
	public:
		Conditional();
		~Conditional();

	public:
		/**
		 * @brief 信号通知下一等待线程
		 */
		void SignalOne() {
			m_hCond.notify_one();
		}

		/**
		 * @brief 信号通知所有等待线程
		 */
		void SignalAll() {
			m_hCond.notify_all();
		}

		/**
		 * @brief 等待条件变量
		 * @param mtx 同步锁
		 */
		void Wait(AutoLocker<Mutex>& mtx) {
			m_hCond.wait(mtx);
		}

		/**
		 * @brief 带超时的等待条件变量
		 * @param mtx 同步锁
		 * @param timeout 毫秒超时时间
		 * @return 返回执行结果
		 */
		bool TimedWait(AutoLocker<Mutex>& mtx, int timeout) {
			return m_hCond.wait_for(mtx, std::chrono::milliseconds(timeout)) != std::_Cv_status::timeout;
		}

	private:
		std::condition_variable m_hCond; ///< 条件变量句柄
	};

	/**
	 * @brief 信号量
	 */
	class Semaphore : public NoCopyable {
	public:
		Semaphore(int init_count = 0);
		~Semaphore();

	public:
		/**
		 * @brief 等待信号量
		 * @return 返回执行结果
		 */
		bool Wait();

		/**
		 * @brief 尝试等待信号量
		 * @return 返回执行结果
		 */
		bool TryWait();

		/**
		 * @brief 超时等待信号量
		 * @param timeout 毫秒超时时间
		 * @return 返回执行结果
		 */
		bool TimedWait(int timeout);

		/**
		 * @brief 放置信号量
		 * @return 返回执行结果
		 */
		bool Post();

	private:
		asl_semaphore_ctx_t* m_hSem; ///< 信号量句柄
	};

	/**
	 * @brief 线程类
	 */
	class Thread : public NoCopyable {
	public:
		Thread();
		Thread(std::function<void()> proc, const std::string name = "");

	public:
		/**
		 * @brief 检测线程是否可等待
		 * @return 返回是否可等待
		 */
		bool Joinable() {
			return m_hThread.joinable();
		}

		/**
		 * @brief 等待线程结束
		 */
		void Join() {
			m_hThread.join();
		}

		/**
		 * @brief 分离线程
		 */
		void Detach() {
			m_hThread.detach();
		}

		/**
		 * @brief 挂起线程
		 * @param timeout 毫秒单位睡眠时间
		 */
		static void Sleep(int timeout);

	protected:
		std::string m_strName;		///< 线程名
		std::thread m_hThread;		///< 线程句柄
	};
	typedef std::shared_ptr<Thread> ThreadPtr_t;

	/**
	 * @brief 线程组
	 */
	class ThreadGroup : public NoCopyable {
	public:
		ThreadGroup();
		~ThreadGroup();

	public:
		/**
		 * @brief 启动新线程
		 * @param proc 线程处理函数
		 * @param param 线程处理函数参数
		 * @param stack_size 堆栈大小
		 * @param name 线程名
		 * @return 成功返回线程指针，失败返回NULL
		 */
		ThreadPtr_t CreateThread(std::function<void()> proc, const std::string name = "");

		/**
		 * @brief 添加线程
		 * @param thread 线程指针
		 */
		void AddThread(ThreadPtr_t thread);

		/**
		 * @brief 移除线程
		 * @param thread 线程指针
		 */
		void RemoveThread(ThreadPtr_t thread);

		/**
		 * @brief 结束并删除线程
		 * @param thread 线程指针
		 */
		void DeleteThread(ThreadPtr_t thread);

		/**
		 * @brief 等待并删除所有线程
		 */
		void DeleteAll();

		/**
		 * @brief 检测线程是否在线程组
		 * @param thread 线程指针
		 * @return 返回检测结果
		 */
		bool IsThreadIn(ThreadPtr_t thread);

	private:
		Mutex m_mtxThreadsLock;				///< 线程组同步锁
		std::vector<ThreadPtr_t> m_lstThreads;	///< 线程列表
	};

	/**
	 * @brief 线程池
	 */
	class ThreadPool : public NoCopyable {
	public:
		ThreadPool();
		~ThreadPool();

		typedef std::function<void ()> FuncTaskProc_t;	///< 任务处理函数
		typedef asl_thread_pool_ctx_t ThreadContext_t;

	public:
		/**
		 * @brief 创建线程池
		 * @param max_thread_num 最大工作线程数
		 * @return 返回执行结果
		 */
		bool Create(int max_thread_num);

		/**
		 * @brief 释放线程池
		 */
		void Release();

		/**
		 * @brief 异步执行任务
		 * @param proc 任务处理函数
		 * @param timeout 毫秒超时时间
		 * @return 返回执行结果
		 */
		bool Exec(FuncTaskProc_t proc, int timeout = -1);

	private:
		/**
		 * @brief 工作线程处理函数
		 * @param context 线程上下文
		 */
		void _WorkerThreadProc(ThreadContext_t* context);

		ThreadContext_t* _AllocThreadContext();
		void _FreeThreadContext(ThreadContext_t* context);
		void _ReleaseFreeThreads();

	private:
		int m_nMaxThreadNum;			///< 最大工作线程数
		bool m_bWorking;				///< 线程池工作状态
		ThreadGroup m_tgThreadGroup;	///< 工作线程组
		Semaphore* m_pFreeThreadSem;	///< 空闲线程列表信号量
		Mutex m_mtxFreeThreadListLock;	///< 空闲线程列表锁
		std::deque<ThreadContext_t*> m_lstFreeThreadList;		///< 空闲线程列表
	};
}
