/**
* @file json_test.cpp
* @brief JSON单元测试
* @author 程行通
*/

#include "asl/utest.hpp"
#include "asl/convert.hpp"
#include "asl/json.hpp"
#include <sstream>
#include <iostream>

using namespace ASL_NAMESPACE;

ASL_UTEST_SUITE(json)

//#define DEBUG_PRINT

ASL_UTEST_CASE(json, null_test) {
	{
		const char* szSrc = "null";
        JsonNode* pNode = JsonNode::Parse(szSrc);
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Null);
        delete pNode;
	}
	{
        JsonNode* pNode = new JsonNull();
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		std::stringstream ss;
		pNode->Print(ss);
        delete pNode;

		pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Null);
        delete pNode;
	}
	{
        JsonNode* pNode = new JsonNull();
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
		pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Null);
        delete pNode;
	}
}

ASL_UTEST_CASE(json, bool_test) {
	{
        const char* szSrc = "true";
		JsonNode* pNode = JsonNode::Parse(szSrc);
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_True);
        delete pNode;
	}
	{
        const char* szSrc = "false";
		JsonNode* pNode = JsonNode::Parse(szSrc);
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_False);
        delete pNode;
	}
	{
        JsonNode* pNode = new JsonBool(true);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_True);
        delete pNode;
	}
    {
        JsonNode* pNode = new JsonBool(false);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_False);
        delete pNode;
	}
    {
        JsonNode* pNode = new JsonBool(true);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_True);
        delete pNode;
	}
    {
        JsonNode* pNode = new JsonBool(false);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_False);
        delete pNode;
	}
}

ASL_UTEST_CASE(json, number_test) {
	{
        const char* szSrc = "12345";
		JsonNode* pNode = JsonNode::Parse(szSrc);
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Number);
		ASL_UTEST_ASSERT_EQ((int64_t)*(JsonNumber*)pNode, 12345);
        ASL_UTEST_ASSERT_APPROX_EQ((double)*(JsonNumber*)pNode, (double)12345);
        delete pNode;
	}
	{
        const char* szSrc = "3.14159265358979323846";
		JsonNode* pNode = JsonNode::Parse(szSrc);
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Number);
        ASL_UTEST_ASSERT_EQ((int64_t)*(JsonNumber*)pNode, 3);
        ASL_UTEST_ASSERT_APPROX_EQ((double)*(JsonNumber*)pNode, 3.14159265358979323846);
        delete pNode;
	}
	{
        JsonNode* pNode = new JsonNumber(12345);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Number);
        ASL_UTEST_ASSERT_EQ((int64_t)*(JsonNumber*)pNode, 12345);
        ASL_UTEST_ASSERT_APPROX_EQ((double)*(JsonNumber*)pNode, (double)12345);
        delete pNode;
	}
	{
        JsonNode* pNode = new JsonNumber(3.14159265358979323846);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Number);
        ASL_UTEST_ASSERT_EQ((int64_t)*(JsonNumber*)pNode, 3);
        ASL_UTEST_ASSERT_APPROX_EQ((double)*(JsonNumber*)pNode, 3.14159265358979323846);
        delete pNode;
	}
    {
        JsonNode* pNode = new JsonNumber(12345);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Number);
        ASL_UTEST_ASSERT_EQ((int64_t)*(JsonNumber*)pNode, 12345);
        ASL_UTEST_ASSERT_APPROX_EQ((double)*(JsonNumber*)pNode, (double)12345);
        delete pNode;
	}
	{
        JsonNode* pNode = new JsonNumber(3.14159265358979323846);
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Number);
        ASL_UTEST_ASSERT_EQ((int64_t)*(JsonNumber*)pNode, 3);
        ASL_UTEST_ASSERT_APPROX_EQ((double)*(JsonNumber*)pNode, 3.14159265358979323846);
        delete pNode;
	}
}

ASL_UTEST_CASE(json, string_test) {
	{
        std::string strDest = "cadfnliugsc\"fwcse";
		std::string strSrc = "\"cadfnliugsc\\\"fwcse\"";

        JsonNode* pNode = JsonNode::Parse(strSrc.c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
		ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strDest);
        delete pNode;
	}
	{
		std::string strDest = (const char*)from_widebyte_cast<ASCID_ANSI>(L"打完偶第三次内裤");
		std::string strSrc = "\"" + strDest + "\"";

        JsonNode* pNode = JsonNode::Parse(strSrc.c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strDest);
        delete pNode;
	}
	{
		std::string strDest = (const char*)from_widebyte_cast<ASCID_UTF8>(L"打完偶第三次内裤");
		std::string strSrc = "\"" + strDest + "\"";

        JsonNode* pNode = JsonNode::Parse(strSrc.c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strDest);
        delete pNode;
	}
	{
        std::string strSrc = "cadfnliugsc\"fwcse";
        JsonNode* pNode = new JsonString(strSrc.c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strSrc);
        delete pNode;
	}
	{
		std::string strSrc = (const char*)from_widebyte_cast<ASCID_ANSI>(L"打完偶第三次内裤");
        JsonNode* pNode = new JsonString(strSrc.c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strSrc);
        delete pNode;
	}
	{
		std::string strSrc = (const char*)from_widebyte_cast<ASCID_UTF8>(L"打完偶第三次内裤");
        JsonNode* pNode = new JsonString(strSrc.c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strSrc);
        delete pNode;
	}
	{
        std::string strSrc = "deyaefg7638yqihu3g8q2632ftw78e96";
        JsonNode* pNode = new JsonString(strSrc.c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strSrc);
        delete pNode;
	}
	{
        std::string strSrc = "deyaefg7638yqihu3g8q2632ftw78e96";
        JsonNode* pNode = new JsonString(strSrc.c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
        pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_String);
        ASL_UTEST_ASSERT_EQ((std::string)*(JsonString*)pNode, strSrc);
        delete pNode;
	}
}

ASL_UTEST_CASE(json, array_test) {
    JsonArray* pSrc = new JsonArray();
    ASL_UTEST_ASSERT_NE(pSrc, (JsonNode*)NULL);

	for(int i = 0; i < 5; ++i) {
		pSrc->AddItem(new JsonNumber(3 * i + 1));
	}

    std::stringstream ss1, ss2;
	pSrc->Print(ss1);
	pSrc->PrintUnformatted(ss2);

#ifdef DEBUG_PRINT
	std::cout << ss1.str() << std::endl;
	std::cout << ss2.str() << std::endl;
#endif

	{
        JsonNode* pNode = JsonNode::Parse(ss1.str().c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Array);
		JsonArray* pArray = (JsonArray*)pNode;

		int nCount = pArray->GetItemCount();
		ASL_UTEST_ASSERT_EQ(nCount, 5);
		for(int i = 0; i < nCount; ++i) {
			JsonNode* pItem = pArray->GetItem(i);
            ASL_UTEST_ASSERT_NE(pItem, (JsonNode*)NULL);
			ASL_UTEST_ASSERT_EQ(pItem->GetType(), JNT_Number);
			ASL_UTEST_ASSERT_EQ((int)*(JsonNumber*)pItem, 3 * i + 1)
		}

        delete pNode;
	}
	{
        JsonNode* pNode = JsonNode::Parse(ss2.str().c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Array);
        JsonArray* pArray = (JsonArray*)pNode;

        int nCount = pArray->GetItemCount();
		ASL_UTEST_ASSERT_EQ(nCount, 5);
		for(int i = 0; i < nCount; ++i) {
			JsonNode* pItem = pArray->GetItem(i);
            ASL_UTEST_ASSERT_NE(pItem, (JsonNode*)NULL);
            ASL_UTEST_ASSERT_EQ(pItem->GetType(), JNT_Number);
            ASL_UTEST_ASSERT_EQ((int)*(JsonNumber*)pItem, 3 * i + 1)
		}

        delete pNode;
	}
	{
        JsonNode* pNode = JsonNode::Parse(ss1.str().c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		std::stringstream ss;
		pNode->Print(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Array);
        delete pNode;
	}
	{
		JsonNode* pNode = JsonNode::Parse(ss1.str().c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        std::stringstream ss;
		pNode->PrintUnformatted(ss);
        delete pNode;

        pNode = JsonNode::Parse(ss.str().c_str());
		ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Array);
        delete pNode;
	}

    delete pSrc;
}

ASL_UTEST_CASE(json, object_test) {
    JsonObject* pSrc = new JsonObject();
    ASL_UTEST_ASSERT_NE(pSrc, (JsonObject*)NULL);

	for(int i = 0; i < 5; ++i) {
		pSrc->SetItem(to_string_cast<int>(i), new JsonNumber(3 * i + 1));
	}

    std::stringstream ss1, ss2;
	pSrc->Print(ss1);
	pSrc->PrintUnformatted(ss2);

#ifdef DEBUG_PRINT
	std::cout << ss1.str(); << std::endl;
	std::cout << ss2.str() << std::endl;
#endif

	{
        JsonNode* pNode = JsonNode::Parse(ss1.str().c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
		ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Object);
		JsonObject* pObject = (JsonObject*)pNode;

		int nCount = pObject->GetItemCount();
		ASL_UTEST_ASSERT_EQ(nCount, 5);
		for(int i = 0; i < nCount; ++i) {
			JsonNode* pItem = pObject->GetItem(to_string_cast<int>(i));
            ASL_UTEST_ASSERT_NE(pItem, (JsonNode*)NULL);
			ASL_UTEST_ASSERT_EQ(pItem->GetType(), JNT_Number);
			ASL_UTEST_ASSERT_EQ((int)*(JsonNumber*)pItem, 3 * i + 1)
		}

        delete pNode;
	}

	{
		JsonNode* pNode = JsonNode::Parse(ss2.str().c_str());
        ASL_UTEST_ASSERT_NE(pNode, (JsonNode*)NULL);
        ASL_UTEST_ASSERT_EQ(pNode->GetType(), JNT_Object);
        JsonObject* pObject = (JsonObject*)pNode;

        int nCount = pObject->GetItemCount();
		ASL_UTEST_ASSERT_EQ(nCount, 5);
		for(int i = 0; i < nCount; ++i) {
            JsonNode* pItem = pObject->GetItem(to_string_cast<int>(i));
            ASL_UTEST_ASSERT_NE(pItem, (JsonNode*)NULL);
            ASL_UTEST_ASSERT_EQ(pItem->GetType(), JNT_Number);
            ASL_UTEST_ASSERT_EQ((int)*(JsonNumber*)pItem, 3 * i + 1)
		}

        delete pNode;
	}

    delete pSrc;
}

/*
ASL_UTEST_CASE(json, notes_test)
{
	const char* szSrc1 = "{\n\t\"a\":1,\r\n\t\"b\":2,\n\r\t\"c\":3\n}";
	const char* szSrc2 = "{\n\t//fszrserge\n\t\"a\":1,\r\n\t//fsezresef\r\n\t\"b\":2,\n\r\t//我的神啊\n\r\t\"c\":3\n}";

	asl_json_t* pRoot1 = asl_json_parse(szSrc1);
	ASL_UTEST_ASSERT_NE(pRoot1, (asl_json_t*)NULL);
	asl_json_t* pRoot2 = asl_json_parse(szSrc2);
	ASL_UTEST_ASSERT_NE(pRoot2, (asl_json_t*)NULL);

    char* pStr1 = asl_json_print(pRoot1);
    ASL_UTEST_ASSERT_NE(pStr1, (char*)NULL);
    char* pStr2 = asl_json_print(pRoot2);
    ASL_UTEST_ASSERT_NE(pStr2, (char*)NULL);

	ASL_UTEST_ASSERT_EQ(strcmp(pStr1, pStr2), 0);

    asl_free(pStr1);
    asl_free(pStr2);

    asl_json_delete(pRoot1);
    asl_json_delete(pRoot2);
}
*/
