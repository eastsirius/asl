/**
* @file convert_test.cpp
* @brief 类型转换单元测试
* @author 程行通
*/

#include "asl/utest.hpp"
#include "asl/convert.hpp"

using namespace ASL_NAMESPACE;

ASL_UTEST_SUITE(convert)

ASL_UTEST_CASE(convert, string_codec_convert) {
	wchar_t acUTF16Src[] = L"地球人都知道";
    char acUTF8Src[] = {'\xE5', '\x9C', '\xB0', '\xE7', '\x90', '\x83', '\xE4', '\xBA', '\xBA', '\xE9', '\x83', '\xBD', '\xE7', '\x9F',  '\xA5', '\xE9', '\x81', '\x93', '\x00'};
//#ifdef WINDOWS
    char acGB2312Src[] = {'\xB5', '\xD8', '\xC7', '\xF2', '\xC8', '\xCB', '\xB6', '\xBC', '\xD6', '\xAA', '\xB5', '\xC0', '\x00'};
    char* acLocalSrc = acGB2312Src;
//#else
//    char* acLocalSrc = acUTF8Src;
//#endif
    
	{
        std::string strValue = (char*)from_widebyte_cast<ASCID_UTF8>(acUTF16Src);
		ASL_UTEST_ASSERT_EQ(strValue, acUTF8Src);
        std::wstring wstrValue = (wchar_t*)to_widebyte_cast<ASCID_UTF8>(acUTF8Src);
		ASL_UTEST_ASSERT_EQ(wstrValue, acUTF16Src);
	}

	{
        std::string strValue = (char*)from_widebyte_cast<ASCID_ANSI>(acUTF16Src);
		ASL_UTEST_ASSERT_EQ(strValue, acLocalSrc);
        std::wstring wstrValue = (wchar_t*)to_widebyte_cast<ASCID_ANSI>(acLocalSrc);
		ASL_UTEST_ASSERT_EQ(wstrValue, acUTF16Src);
	}
}

ASL_UTEST_CASE(convert, type_convert_cpp) {
	{
		typedef int32_t TestType_t;
		std::string strSrc = "-2635";
		TestType_t vSrc = -2635;
		TestType_t vValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(vValue, vSrc);
        std::string strValue = (char*)to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(strValue, strSrc);
	}

	{
		typedef float TestType_t;
		std::string strSrc = "3.1415";
		TestType_t vSrc = (TestType_t)3.1415;
		TestType_t vValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(vValue, vSrc);
        std::string strValue = (char*)to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(from_string_cast<TestType_t>(strValue), vSrc);
	}

	{
		typedef unsigned int TestType_t;
		std::string strSrc = "43513";
		TestType_t vSrc = 43513;
		TestType_t nValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(nValue, vSrc);
        std::string strValue = (char*)to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(strValue, strSrc);
	}

	{
		typedef int64_t TestType_t;
		std::string strSrc = "-43513";
		TestType_t vSrc = -43513;
		TestType_t nValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(nValue, vSrc);
        std::string strValue = (char*)to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(strValue, strSrc);
	}

	{
		typedef uint64_t TestType_t;
		std::string strSrc = "43513";
		TestType_t vSrc = 43513;
		TestType_t nValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(nValue, vSrc);
        std::string strValue = (char*)to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(strValue, strSrc);
	}

	{
		typedef bool TestType_t;
		std::string strSrc = "true";
		TestType_t vSrc = true;
		TestType_t nValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(nValue, vSrc);
        std::string strValue = (char*)to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(strValue, strSrc);

		strSrc = "false";
		vSrc = false;
		nValue = from_string_cast<TestType_t>(strSrc);
		ASL_UTEST_ASSERT_EQ(nValue, vSrc);
		strValue = to_string_cast<TestType_t>(vSrc);
		ASL_UTEST_ASSERT_EQ(strValue, strSrc);
	}
}
