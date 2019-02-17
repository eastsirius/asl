/**
 * @file convert.cpp
 * @brief 转换工具
 * @author 程行通
 */

#include "convert.hpp"
#ifndef WINDOWS
#  include <iconv.h>
#else
#  include <windows.h>
#endif

#ifdef UNIX
#  ifdef APPLE
#    define ASL_STRING_CODE_WCHAR "UTF-32LE"
#    define ASL_STRING_CODE_UTF8  "UTF-8"
#    define ASL_STRING_CODE_ANSI  "GB2312//TRANSLIT"
#  else
#    define ASL_STRING_CODE_WCHAR "WCHAR_T"
#    define ASL_STRING_CODE_UTF8  "UTF-8"
#    define ASL_STRING_CODE_ANSI  "GB2312//TRANSLIT"
#  endif
#endif

namespace ASL_NAMESPACE {
	namespace helper {
		int ConvertHelper::GetWStringLength(const wchar_t *str) {
			int nRet = 0;
			while(str[nRet] != 0) {
				++nRet;
			}

			return nRet;
		}

		std::string UnicodeToAnsi(const wchar_t* str) {
			assert(str != NULL);
			std::string ret;
#ifdef WINDOWS
			char *pacBuffer = NULL;

			int bufferLength = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, 0, NULL);
			if(bufferLength > 0) {
				pacBuffer = new char[bufferLength + 1];
				WideCharToMultiByte(CP_ACP, 0, str, -1, pacBuffer, bufferLength, 0, NULL);
				pacBuffer[bufferLength] = 0;
				ret = std::string(pacBuffer);
				delete[] pacBuffer;
			}
#else
			iconv_t cd;
			cd = iconv_open(ASL_STRING_CODE_ANSI, ASL_STRING_CODE_WCHAR);
			if(cd == NULL) {
				return "";
			}
			size_t nSrcLength = (helper::ConvertHelper::GetWStringLength(str) + 1) * sizeof(wchar_t);
			size_t nDestLength = nSrcLength;
			char *pDestBuffer = new char[nDestLength];
			char *to = pDestBuffer;
			if((int)iconv(cd, (char**)&str, &nSrcLength, &to, &nDestLength) != -1) {
				ret = pDestBuffer;
			}

			delete[] pDestBuffer;
			iconv_close(cd);
#endif
			return ret;
		}

		std::wstring ConvertHelper::AnsiToUnicode(const char* str) {
			assert(str != NULL);
			std::wstring ret;
#ifdef WINDOWS
			wchar_t *pacwBuffer = NULL;

			int wbufferLength = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
			if(wbufferLength > 0) {
				pacwBuffer = new wchar_t[wbufferLength + 1];
				memset(pacwBuffer, 0, wbufferLength + 1);
				if(MultiByteToWideChar(CP_ACP, 0, str, -1, pacwBuffer, wbufferLength) != wbufferLength) {
					delete[] pacwBuffer;
					return L"";
				}
				ret = std::wstring(pacwBuffer);
				delete[] pacwBuffer;
			}
#else
			iconv_t cd;
			cd = iconv_open(ASL_STRING_CODE_WCHAR, ASL_STRING_CODE_ANSI);
			if(cd == NULL) {
				return L"";
			}
			size_t nSrcLength = strlen(str) + 1;
			size_t nDestLength = nSrcLength * sizeof(wchar_t);
			wchar_t *pDestBuffer = new wchar_t[nDestLength];
			wchar_t *to = pDestBuffer;
			if((int)iconv(cd, (char**)&str, &nSrcLength, (char**)&to, &nDestLength) != -1) {
				ret = pDestBuffer;
			}
			delete[] pDestBuffer;
			iconv_close(cd);
#endif
			return ret;
		}

		std::string UnicodeToUtf8(const wchar_t* str) {
			assert(str != NULL);
			std::string ret;
#ifdef WINDOWS
			char *pacBuffer = NULL;

			int bufferLength = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, 0, NULL);
			if(bufferLength > 0) {
				pacBuffer = new char[bufferLength + 1];
				memset(pacBuffer, 0, bufferLength + 1);
				if(WideCharToMultiByte(CP_UTF8, 0, str, -1, pacBuffer, bufferLength, 0, NULL) != bufferLength) {
					delete[] pacBuffer;
					return "";
				}
				ret = std::string(pacBuffer);
				delete[] pacBuffer;
			}
#else
			iconv_t cd;
			cd = iconv_open(ASL_STRING_CODE_UTF8, ASL_STRING_CODE_WCHAR);
			if(cd == NULL) {
				return "";
			}
			size_t nSrcLength = (helper::ConvertHelper::GetWStringLength(str) + 1) * sizeof(wchar_t);
			size_t nDestLength = nSrcLength;
			char *pDestBuffer = new char[nDestLength];
			char *to = pDestBuffer;
			if((int)iconv(cd, (char**)&str, &nSrcLength, &to, &nDestLength) != -1) {
				ret = pDestBuffer;
			}

			delete[] pDestBuffer;
			iconv_close(cd);
#endif
			return ret;
		}

		std::wstring Utf8ToUnicode(const char* str) {
			assert(str != NULL);
			std::wstring ret;
#ifdef WINDOWS
			wchar_t *pacwBuffer = NULL;

			int wbufferLength = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
			if(wbufferLength > 0) {
				pacwBuffer = new wchar_t[wbufferLength + 1];
				MultiByteToWideChar(CP_UTF8, 0, str, -1, pacwBuffer, wbufferLength);
				pacwBuffer[wbufferLength] = 0;
				ret = std::wstring(pacwBuffer);
				delete[] pacwBuffer;
			}
#else
			iconv_t cd;
			cd = iconv_open(ASL_STRING_CODE_WCHAR, ASL_STRING_CODE_UTF8);
			if(cd == NULL) {
				return L"";
			}
			size_t nSrcLength = strlen(str) + 1;
			size_t nDestLength = nSrcLength * sizeof(wchar_t);
			wchar_t *pDestBuffer = new wchar_t[nDestLength];
			wchar_t *to = pDestBuffer;
			if((int)iconv(cd, (char**)&str, &nSrcLength, (char**)&to, &nDestLength) != -1) {
				ret = pDestBuffer;
			}

			delete[] pDestBuffer;
			iconv_close(cd);
#endif
			return ret;
		}

		static inline char _ToHexChar(int v, bool up_case) {
			if(v < 10) {
				return char(v + '0');
			} else {
				if(up_case) {
					return char(v - 10 + 'A');
				} else {
					return char(v - 10 + 'a');
				}
			}
		}
	}

	void DataToHexString(std::string& dest, const uint8_t* data, int size, bool up_case) {
		dest = "";
		for(int i = 0; i < size; ++i) {
			int c = data[i];
			dest.push_back(helper::_ToHexChar(c >> 4, up_case));
			dest.push_back(helper::_ToHexChar(c & 0xF, up_case));
		}
	}

	std::string DataToHexString(const uint8_t* data, int size, bool up_case) {
		std::string strDest;
		DataToHexString(strDest, data, size, up_case);
		return strDest;
	}
}
