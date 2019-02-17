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
