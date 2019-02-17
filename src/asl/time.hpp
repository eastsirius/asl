/**
 * @file time.hpp
 * @brief 时间工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <stdint.h>
#include <time.h>

namespace ASL_NAMESPACE {
	/**
	 * @brief 时间类
	 */
	class Time {
	public:
		Time();
		Time(int64_t value);

	public:
		/**
		 * @brief 获取微秒时间值
		 * @return 返回微秒时间值
		 */
		int64_t Value() const {
			return m_s64MicrosecTime;
		}

	public:
		/**
		 * @brief 获取微秒时间
		 * @return 返回微秒时间
		 */
		static Time GetTime();

	private:
		int64_t m_s64MicrosecTime;
	};

	/**
	 * @brief 日期时间
	 */
	class Datetime {
	public:
		Datetime();

	public:
		/**
		 * @brief 获取年份
		 * @return 返回年份
		 */
		int GetYear() const {
			return m_nYear;
		}

		/**
		 * @brief 获取月份
		 * @return 返回月份
		 */
		int GetMonth() const {
			return m_nMonth;
		}

		/**
		 * @brief 获取日期
		 * @return 返回日期
		 */
		int GetDay() const {
			return m_nDay;
		}

		/**
		 * @brief 获取周几
		 * @return 返回周几
		 */
		int GetDayOfWeek() const {
			return m_nDayOfWeek;
		}

		/**
		 * @brief 获取小时数
		 * @return 返回小时数
		 */
		int GetHour() const {
			return m_nHour;
		}

		/**
		 * @brief 获取分钟数
		 * @return 返回分钟数
		 */
		int GetMinute() const {
			return m_nMinute;
		}

		/**
		 * @brief 获取秒数
		 * @return 返回秒数
		 */
		int GetSecond() const {
			return m_nSecond;
		}

		/**
		 * @brief 获取毫秒数
		 * @return 返回微秒数
		 */
		int GetMillisecond() const {
			return m_nMilliseconds;
		}

		/**
		 * @brief 获取微秒数
		 * @return 返回微秒数
		 */
		int GetMicrosecond() const {
			return m_nMicroseconds;
		}

	public:
		/**
		 * @brief 获取系统时间(格林时间)
		 * @return 返回系统时间
		 */
		static Datetime GetSystemTime();

		/**
		 * @brief 获取本地时间
		 * @return 返回本地时间
		 */
		static Datetime GetLocalTime();

		/**
		 * @brief 转换到系统时间(格林时间)
		 * @return 返回系统时间
		 */
		static Datetime ToSystemTime(Time value);

		/**
		 * @brief 转换到本地时间
		 * @return 返回本地时间
		 */
		static Datetime ToLocalTime(Time value);

	private:
		/**
		 * @brief 设置日期时间
		 * @param value 时间值
		 * @param dt 日期
		 */
		void setDatetime(Time value, tm* dt);

	private:
		Time m_tValue;
		int m_nYear;
		int m_nMonth;
		int m_nDayOfWeek;
		int m_nDay;
		int m_nHour;
		int m_nMinute;
		int m_nSecond;
		int m_nMilliseconds;
		int m_nMicroseconds;
	};

	/**
	 * @brief 计时器
	 */
	class Timer {
	public:
		Timer();

	public:
		/**
		 * @brief 重启计时器
		 */
		void Restart();

		/**
		 * @brief 获取当前时间(s)
		 * @return 返回当前时间
		 */
		int64_t SecTime() const;

		/**
		 * @brief 获取当前时间(ms)
		 * @return 返回当前时间
		 */
		int64_t MillisecTime() const;

		/**
		 * @brief 获取当前时间(us)
		 * @return 返回当前时间
		 */
		int64_t MicrosecTime() const;

	private:
		Time m_tBeginTime;
	};

	/**
	 * @class 获取毫秒时间值
	 * @brief 返回毫秒时间值
	 */
	int64_t asl_get_ms_time();

	/**
	 * @class 获取微秒时间值
	 * @brief 返回微秒时间值
	 */
	int64_t asl_get_us_time();
}
