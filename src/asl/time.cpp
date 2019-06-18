/**
 * @file time.cpp
 * @brief 时间工具
 * @author 程行通
 */

#include "time.hpp"
#include "utils.hpp"
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
		st.wYear -= 369;
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


	static const int minYear = 0;
	static const int maxYear = 9999;
	static const int minMonth = 1;
	static const int maxMonth = 12;
	static const int minDay = 1;
	static const int maxDay[12] = {
			31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	static const int maxDayFebruaryLeapYear = 29;
	static const int minHour = 0;
	static const int maxHour = 23;
	static const int minMinute = 0;
	static const int maxMinute = 59;
	static const int minSecond = 0;
	static const int maxSecond = 59;
	static const int minMilliSec = 0;
	static const int maxMilliSec = 999;
	static const int minNumOffsetHour = -12;
	static const int maxNumOffsetHour = 12;
	static const char dateSeparation = '-';
	static const char dateTimeSeparation = 'T';
	static const char timeSeparation = ':';
	static const char msSeparation = '.';
	static const char timeUtcOffset = 'Z';
	static const char timeNumOffsetPlus = '+';
	static const char timeNumOffsetMinus = '-';
	static const int dateYearIndex = 4;
	static const int dateMonthIndex = 7;
	static const int dateTimeIndex = 10;
	static const int timeHourIndex = 13;
	static const int timeMinuteIndex = 16;
	static const int timeSecondIndex = 19;
	static const int timeNumOffsetIndex = 22;

	Rfc3339::Rfc3339() : m_bSummerTime(false), m_bLocalTime(false) {
	}

	int64_t Rfc3339::Parse(const char* szStr) {
		std::string rfc3339Date(szStr);

		bool bMilliSec = false;
		bool utcOffset = false;
		if(!_ValidateFormat(rfc3339Date, bMilliSec, utcOffset)) {
			return -1;
		}

		int year = atoi(rfc3339Date.substr(0, dateYearIndex).c_str());
		int month = atoi(rfc3339Date.substr(dateYearIndex + 1, dateMonthIndex - dateYearIndex - 1).c_str());
		int day = atoi(rfc3339Date.substr(dateMonthIndex + 1, dateTimeIndex - dateMonthIndex - 1).c_str());
		int hour = atoi(rfc3339Date.substr(dateTimeIndex + 1, timeHourIndex - dateTimeIndex - 1).c_str());
		int minute = atoi(rfc3339Date.substr(timeHourIndex + 1, timeMinuteIndex - timeHourIndex - 1).c_str());
		int second = atoi(rfc3339Date.substr(timeMinuteIndex + 1, timeSecondIndex - timeMinuteIndex - 1).c_str());
		int millisec = 0, mslen = 0;
		if(bMilliSec) {
			mslen = 4;
			millisec = atoi(rfc3339Date.substr(timeSecondIndex + 1, mslen - 1).c_str());
		}

		int numOffsetHour = 0;
		int numOffsetMinute = 0;
		if(false == utcOffset) {
			numOffsetHour = atoi(rfc3339Date.substr(timeSecondIndex + mslen, timeNumOffsetIndex - timeSecondIndex).c_str());
			numOffsetMinute = atoi(rfc3339Date.substr(timeNumOffsetIndex + mslen + 1).c_str());
		}

		if(!_ValidateData(year, month, day, hour, minute, second, millisec, numOffsetHour, numOffsetMinute)) {
			return -1;
		}

		tm gt;
		gt.tm_year = year - 1900;
		gt.tm_mon = month - 1,
		gt.tm_mday = day,
		gt.tm_hour = hour;
		gt.tm_min = minute;
		gt.tm_sec = second;
		//if(utcOffset) {
			gt.tm_hour -= numOffsetHour;
		//}

#ifdef WINDOWS
		return 1000 * _mkgmtime(&gt) + millisec;
#else
		return 1000 * timegm(&gt) + millisec;
#endif
	}

	std::string Rfc3339::Print(int64_t nMilliSecTime) {
		time_t date = nMilliSecTime / 1000;
		int ms = nMilliSecTime % 1000;
		tm *gt;
		gt = gmtime(&date);
		int tmp = gt->tm_hour;
		if(m_bLocalTime) {
			tm *lt;
			lt = localtime(&date);
			int offset = lt->tm_hour - tmp;
			if(offset > 12) {
				offset -= 24;
			} else if(offset <= -12) {
				offset += 24;
			}

			char dateString[32];
			snprintf(dateString, 32, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d%c%03d%+03d%c00",
					 lt->tm_year + 1900,
					 dateSeparation,
					 lt->tm_mon + 1,
					 dateSeparation,
					 lt->tm_mday,
					 dateTimeSeparation,
					 lt->tm_hour,
					 timeSeparation,
					 lt->tm_min,
					 timeSeparation,
					 lt->tm_sec,
					 msSeparation,
					 ms,
					 offset, // number offset
					 timeSeparation
			);
			return std::string(dateString);
		} else {
			char dateString[32];
			snprintf(dateString, 32, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d%c%03d%c",
					 gt->tm_year + 1900,
					 dateSeparation,
					 gt->tm_mon + 1,
					 dateSeparation,
					 gt->tm_mday,
					 dateTimeSeparation,
					 gt->tm_hour,
					 timeSeparation,
					 gt->tm_min,
					 timeSeparation,
					 gt->tm_sec,
					 msSeparation,
					 ms,
					 timeUtcOffset
			);
			return std::string(dateString);
		}
	}

	void Rfc3339::SetSummerTime(bool bSummerTime) {
		m_bSummerTime = bSummerTime;
	}

	void Rfc3339::SetLocalTime(bool bLocalTime) {
		m_bLocalTime = bLocalTime;
	}

	bool Rfc3339::IsLeapYear(int nYear) {
		return (nYear % 4 == 0 && (nYear % 100 != 0 || nYear % 400 == 0));
	}

	bool Rfc3339::_ValidateFormat(const std::string& strSrc, bool& bMilliSec, bool& bUtcOffset) {
		if(strSrc.length() <= dateYearIndex || dateSeparation != strSrc.at(dateYearIndex)) {
			return false;
		}
		if(strSrc.length() <= dateMonthIndex || dateSeparation != strSrc.at(dateMonthIndex)) {
			return false;
		}
		if(strSrc.length() <= dateTimeIndex || dateTimeSeparation != strSrc.at(dateTimeIndex)) {
			return false;
		}
		if(strSrc.length() <= timeHourIndex || timeSeparation != strSrc.at(timeHourIndex)) {
			return false;
		}
		if(strSrc.length() <= timeMinuteIndex || timeSeparation != strSrc.at(timeMinuteIndex)) {
			return false;
		}

		int nMsLen = 0;
		if(strSrc.length() <= timeSecondIndex) {
			return false;
		}
		if(msSeparation == strSrc.at(timeSecondIndex)) {
			bMilliSec = true;
			nMsLen = 4;
		}

		if(strSrc.length() <= timeSecondIndex + nMsLen) {
			return false;
		}
		if(timeUtcOffset == strSrc.at(timeSecondIndex + nMsLen)) {
			bUtcOffset = true;
		}
		else {
			if(strSrc.length() <= timeNumOffsetIndex + nMsLen) {
				return false;
			}
			if(timeNumOffsetPlus == strSrc.at(timeSecondIndex + nMsLen)) {
				bUtcOffset = false;
			} else if(timeNumOffsetMinus == strSrc.at(timeSecondIndex + nMsLen)) {
				bUtcOffset = false;
			} else {
				return false;
			}
		}

		if(!bUtcOffset) {
			if(timeSeparation != strSrc.at(timeNumOffsetIndex + nMsLen)) {
				return false;
			}
		}
		return true;
	}

	bool Rfc3339::_ValidateData(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
			int nMilliSec, int nNumOffsetHour, int nNumOffsetMinute) {
		if((nYear < minYear) || (maxYear < nYear)) {
			return false;
		}
		if((nMonth < minMonth) || (maxMonth < nMonth)) {
			return false;
		}
		if(IsLeapYear(nYear) && (2 == nMonth)) {
			if((nDay < minDay) || (maxDayFebruaryLeapYear < nDay)) {
				return false;
			}
		}
		if((nDay < minDay) || (maxDay[nMonth - 1] == nDay)) {
			return false;
		}

		if((nHour < minHour) || (maxHour < nHour)) {
			return false;
		}
		if((nMinute < minMinute) || (maxMinute < nMinute)) {
			return false;
		}
		if((nSecond < minSecond) || (maxSecond < nSecond)) {
			return false;
		}
		if((nMilliSec < minMilliSec) || (maxMilliSec < nMilliSec)) {
			return false;
		}

		if((nNumOffsetHour < minNumOffsetHour) || (maxNumOffsetHour < nNumOffsetHour)) {
			return false;
		}
		//if((numOffsetMinute < minMinute) || (maxMinute < numOffsetMinute)) {
		//	return false;
		//}
		if(0 != nNumOffsetMinute) {
			return false;
		}

		return true;
	}


	int64_t asl_get_ms_time() {
		return asl_get_us_time() / 1000;
	}

	int64_t asl_get_us_time() {
		return Time::GetTime().Value();
	}
}
