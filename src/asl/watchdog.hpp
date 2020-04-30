/**
 * @file watchdog.hpp
 * @brief 看门狗
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <functional>

namespace ASL_NAMESPACE {
	/**
	 * @brief 看门狗
	 */
	class WatchDog : public NoCopyable {
	public:
		WatchDog();
		~WatchDog();

	public:
		/**
		 * @brief 开启看门狗
		 * @param nMsTimeout 毫秒超时时间
		 * @param funHandler 超时处理函数
		 */
		void Start(int nMsTimeout, std::function<void()> funHandler);

		/**
		 * @brief 停止看门狗
		 */
		void Stop();

		/**
		 * @brief 复位计时器
		 */
		void Reset();

	private:
	};
}
