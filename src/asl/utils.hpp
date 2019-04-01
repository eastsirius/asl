/**
 * @file utils.hpp
 * @brief 实用工具集
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <string>
#include <cstring>

#define ASL_C2CPPBOOL(v) ((v) != FALSE)
#define ASL_CPP2CBOOL(v) ((v) ? TRUE : FALSE)

#ifdef WINDOWS
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#  define snprintf _snprintf
#endif

#define ASL_MAX(x,y) ((x)>(y)?(x):(y))
#define ASL_MIN(x,y) ((x)<(y)?(x):(y))

#define ASL_ONEPARAM(args1, ...) args1,##__VA_ARGS__

#define ASL_INIT_ONCE_PROC(proc_name, init_proc, sleep_proc) \
	static int proc_name { \
		static int init_count = 0; \
		static bool init_ok = false; \
		static int init_ret = 0; \
		\
		int cnt = ++init_count; \
		if(cnt == 1) { \
			init_ret = init_proc; \
			init_ok = true; \
		} else { \
			while(!init_ok) { \
				sleep_proc; \
			} \
		} \
		\
		return init_ret; \
	} \

namespace ASL_NAMESPACE {
	/**
	 * @brief 取最大值
	 * @return 返回最大值
	 */
	template <typename T>
	static inline T asl_max(T lhs, T rhs) {
		return lhs > rhs ? lhs : rhs;
	}

	/**
	 * @brief 取最小值
	 * @return 返回最小值
	 */
	template <typename T>
	static inline T asl_min(T lhs, T rhs) {
		return lhs < rhs ? lhs : rhs;
	}

	/**
	 * @brief 交换变量
	 * @param lhs 左操作数
	 * @param rhs 右操作数
	 */
	template <typename T>
	static inline void asl_swap(T& lhs, T& rhs) {
		T tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	/**
	 * @brief 获取安全字符串
	 * @return 返回安全字符串
	 */
	static inline const char* asl_safe_str(const char* str) {
		return str ? str : "";
	}

	/**
	 * @brief 二分法查找
	 * @param lst 列表指针(其中元素必须是从小到大排列)
	 * @param size 列表元素数
	 * @param key 待匹配元素
	 * @return 成功返回索引值，失败返回负数
	 * @warning 必须实现 T == keyT, T > keyT 两种操作
	 */
	template <typename T, typename keyT>
	int binary_search(const T* lst, int size, const keyT& key) {
		int high = size - 1, low = 0, midle = size / 2;
		while(high >= low) {
			midle = (high + low) / 2;
			if(lst[midle] == key) {
				return midle;
			}
			if(lst[midle] > key) {
				high = midle - 1;
			} else {
				low = midle + 1;
			}
		}
		
		return -1;
	}

	/**
	 * @brief 不可创建类基类
	 */
	class NoCreatable {
	protected:
		NoCreatable();
	};

	/**
	 * @brief 不可拷贝类基类
	 */
	class NoCopyable {
	public:
		NoCopyable();

	private:
		NoCopyable(const NoCopyable& src);
		const NoCopyable& operator=(const NoCopyable& src);
	};

	/**
	 * @brief 信号管理器
	 */
	class SignalManager : public NoCopyable, public NoCreatable {
	public:
		typedef void (*SignalProc_t)();

		/**
		 * @brief 注册信号
		 * @param stop_proc 停止程序处理函数
		 * @param kill_proc 杀死进程处理函数
		 * @return 返回执行结果
		 */
		bool RegisterSignals(SignalProc_t stop_proc, SignalProc_t kill_proc);

		/**
		 * @brief 反注册
		 */
		void UnregisterSignals();

		/**
		 * @brief 获取唯一实例
		 * @return 返回唯一实例
		 */
		static SignalManager& Instance();

	public:
		/**
		 * @brief 停止进程处理
		 */
		bool OnStop();

		/**
		 * @brief 杀死进程处理
		 */
		bool OnKill();

	private:
		SignalProc_t m_funOnStop;
		SignalProc_t m_funOnKill;
	};
}
