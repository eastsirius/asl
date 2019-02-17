/**
 * @file time.cpp
 * @brief 时间工具
 * @author 程行通
 */

#include "time.hpp"
#ifdef WINDOWS
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

namespace ASL_NAMESPACE {
	Time::Time() : m_s64MicrosecTime(0) {
	}

	Time::Time(int64_t value) : m_s64MicrosecTime(value) {
	}

	Time Time::GetTime() {
#ifdef WINDOWS
		SYSTEMTIME st;
		::GetLocalTime(&st);
		FILETIME ft;
		::SystemTimeToFileTime(&st, &ft);
		return Time(((((uint64_t)ft.dwHighDateTime) << 32) + ft.dwLowDateTime) / 10);
#else
		timeval t;
		gettimeofday(&t, 0);
		return Time(t.tv_sec * 1000000ULL + t.tv_usec);
#endif
	}


	Datetime::Datetime() {
		m_nYear = 0;
		m_nMonth = 0;
		m_nDayOfWeek = 0;
		m_nDay = 0;
		m_nHour = 0;
		m_nMinute = 0;
		m_nSecond = 0;
		m_nMilliseconds = 0;
		m_nMicroseconds = 0;
	}

	Datetime Datetime::GetSystemTime() {
		return ToSystemTime(Time::GetTime());
	}

	Datetime Datetime::GetLocalTime() {
		return ToLocalTime(Time::GetTime());
	}

	Datetime Datetime::ToSystemTime(Time value) {
		time_t tv = (time_t)(value.Value() / 1000000);
		Datetime dt;
		dt.setDatetime(value, gmtime(&tv));
		return dt;
	}

	Datetime Datetime::ToLocalTime(Time value) {
		time_t tv = (time_t)(value.Value() / 1000000);
		Datetime dt;
		dt.setDatetime(value, localtime(&tv));
		return dt;
	}

	void Datetime::setDatetime(Time value, tm* dt) {
		m_nYear = dt->tm_year + 1900;
		m_nMonth = dt->tm_mon + 1;
		m_nDayOfWeek = dt->tm_wday + 1;
		m_nDay = dt->tm_mday;
		m_nHour = dt->tm_hour;
		m_nMinute = dt->tm_min;
		m_nSecond = dt->tm_sec;
		m_nMilliseconds = (int)(value.Value() / 1000 % 1000);
		m_nMicroseconds = (int)(value.Value() % 1000);
	}


	Timer::Timer() {
		Restart();
	}

	void Timer::Restart() {
		m_tBeginTime = Time::GetTime();
	}

	int64_t Timer::SecTime() const {
		return MicrosecTime() / 1000000;
	}

	int64_t Timer::MillisecTime() const {
		return MicrosecTime() / 1000;
	}

	int64_t Timer::MicrosecTime() const {
		return Time::GetTime().Value() - m_tBeginTime.Value();
	}


	int64_t asl_get_ms_time() {
		return asl_get_us_time() / 1000;
	}

	int64_t asl_get_us_time() {
		return Time::GetTime().Value();
	}
}
