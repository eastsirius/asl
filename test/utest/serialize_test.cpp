/**
* @file serialize_test.cpp
* @brief 序列化工具单元测试
* @author 程行通
*/

#include "asl/serialize.hpp"
#include "asl/xml_archive.hpp"
#include "asl/json_archive.hpp"
#include "asl/utest.hpp"
#include <sstream>
#include <ctime>

#define ASL_PI 3.14159265358979323846

using namespace ASL_NAMESPACE;

ASL_UTEST_SUITE(serialize)

//#define DEBUG_PRINT

struct People {
	std::string strName;
	int nAge;
	std::string strAddr;
};

bool operator==(const People& lhs, const People& rhs) {
	return lhs.strName == rhs.strName && lhs.nAge == rhs.nAge && lhs.strAddr == rhs.strAddr;
}

template <class TA>
class AslArchiveSerializer<TA, People> : public asl::ArchiveSerializer<TA, People> {
public:
	void Serialize(TA& a, People& v) {
		a & ASL_ARCHIVABLE_NAMEVALUE("Name", v.strName);
		a & ASL_ARCHIVABLE_NAMEVALUE("Age", v.nAge);
		a & ASL_ARCHIVABLE_NAMEVALUE("Addr", v.strAddr);
	}
};

class People2 {
public:
	std::string strName;
	int nAge;
	std::string strAddr;

	bool operator==(const People2& rhs) const {
		return strName == rhs.strName && nAge == rhs.nAge && strAddr == rhs.strAddr;
	}

	template <class TA>
	void Serialize(TA& a) {
		a & ASL_ARCHIVABLE_NAMEVALUE("Name", strName);
		a & ASL_ARCHIVABLE_NAMEVALUE("Age", nAge);
		a & ASL_ARCHIVABLE_NAMEVALUE("Addr", strAddr);
	}
};


ASL_UTEST_CASE(serialize, bin_serialer) {
	uint8_t ucBuffer[32];

	uint8_t u8Src = 0x7C;
	uint8_t u8Dst = 0;
	BinSerializer<1>::DoWrite(ucBuffer, &u8Src);
	BinSerializer<1>::DoRead(ucBuffer, &u8Dst);
	ASL_UTEST_ASSERT_EQ(u8Src, u8Dst);

	uint16_t u16Src = 0x9DE6;
	uint16_t u16Dst = 0;
	BinSerializer<2>::DoWrite(ucBuffer, &u16Src);
	BinSerializer<2>::DoRead(ucBuffer, &u16Dst);
	ASL_UTEST_ASSERT_EQ(u16Src, u16Dst);

	uint32_t u24Src = 0x4EF6D1;
	uint32_t u24Dst = 0;
	BinSerializer<3>::DoWrite(ucBuffer, &u24Src);
	BinSerializer<3>::DoRead(ucBuffer, &u24Dst);
	ASL_UTEST_ASSERT_EQ(u24Src, u24Dst);

	uint32_t u32Src = 0x8A6E8C9E;
	uint32_t u32Dst = 0;
	BinSerializer<4>::DoWrite(ucBuffer, &u32Src);
	BinSerializer<4>::DoRead(ucBuffer, &u32Dst);
	ASL_UTEST_ASSERT_EQ(u32Src, u32Dst);

	uint64_t u64Src = 0x78BC998ABD9F6639L;
	uint64_t u64Dst = 0;
	BinSerializer<8>::DoWrite(ucBuffer, &u64Src);
	BinSerializer<8>::DoRead(ucBuffer, &u64Dst);
	ASL_UTEST_ASSERT_EQ(u64Src, u64Dst);

	double dbSrc = ASL_PI;
	double dbDst = 0.0;
	BinSerializer<sizeof(double)>::DoWrite(ucBuffer, &dbSrc);
	BinSerializer<sizeof(double)>::DoRead(ucBuffer, &dbDst);
	ASL_UTEST_ASSERT_EQ(dbSrc, dbDst);
}

ASL_UTEST_CASE(serialize, string_serialer) {
	const std::string strSrc = "ndqiuefg7648hfse7giho39wy3ahgm8o3jwegwd376fwa3ff3fzwe3";
	std::string strDst;
	uint8_t ucBuffer[512] = {0};

#define ASL_TESTDEFINE(n) \
	strDst = ""; \
	memset(ucBuffer, 0, sizeof(ucBuffer)); \
	ASL_UTEST_ASSERT_EQ(StringSerializer<n>::DoWrite(ucBuffer, sizeof(ucBuffer), strSrc), int(strSrc.length() + n)); \
	ASL_UTEST_ASSERT_EQ(StringSerializer<n>::DoRead(ucBuffer, sizeof(ucBuffer), strDst), int(strSrc.length() + n)); \
	ASL_UTEST_ASSERT_EQ(strSrc, strDst); \

	ASL_TESTDEFINE(1)
	ASL_TESTDEFINE(2)
	ASL_TESTDEFINE(3)
	ASL_TESTDEFINE(4)

#undef ASL_TESTDEFINE
}

ASL_UTEST_CASE(serialize, xml_archive_simple) {
	std::stringstream ss;
	XmlOutputArchive xout(ss);
	srand((unsigned int)time(NULL));

#define ASL_TESTDEFINE(id, type) \
	type src##id = 0, dst##id = 0; \
	for(size_t i = 0; i < sizeof(type); ++i) { \
		((uint8_t*)&src##id)[i] = (uint8_t)rand(); \
		((uint8_t*)&dst##id)[i] = 0; \
	} \
	xout & ASL_ARCHIVABLE_NAMEVALUE("v" #id, src##id); \

#define ASL_TESTDEFINE2(id, type, s, d) \
	type src##id = 0, dst##id = 0; \
	src##id = (type)s; \
	dst##id = d; \
	xout & ASL_ARCHIVABLE_NAMEVALUE("v" #id, src##id); \

	ASL_TESTDEFINE( 1, char)
	ASL_TESTDEFINE( 2, signed char)
	ASL_TESTDEFINE( 3, unsigned char)
	ASL_TESTDEFINE( 4, short)
	ASL_TESTDEFINE( 5, unsigned short)
	ASL_TESTDEFINE( 6, int)
	ASL_TESTDEFINE( 7, unsigned int)
	ASL_TESTDEFINE( 8, long)
	ASL_TESTDEFINE( 9, unsigned long)
	ASL_TESTDEFINE(10, long long)
	ASL_TESTDEFINE(11, unsigned long long)
	ASL_TESTDEFINE2(12, float, ASL_PI, 0)
	ASL_TESTDEFINE2(13, double, ASL_PI, 0)
	//ASL_TESTDEFINE2(14, long double, ASL_PI, 0)
	ASL_TESTDEFINE2(15, bool, true, false)

#undef ASL_TESTDEFINE2
#undef ASL_TESTDEFINE

	xout.Flush();

#ifdef DEBUG_PRINT
	std::cout << ss.str().c_str() << std::endl;
#endif

	XmlInputArchive xin(ss);

#define ASL_TESTDEFINE(id, type) \
	xin & ASL_ARCHIVABLE_NAMEVALUE("v" #id, dst##id); \
	ASL_UTEST_ASSERT_EQ(src##id, dst##id); \

#define ASL_TESTDEFINE2(id, type) \
	xin & ASL_ARCHIVABLE_NAMEVALUE("v" #id, dst##id); \
	ASL_UTEST_ASSERT_APPROX_EQ(src##id, dst##id); \
	
	ASL_TESTDEFINE( 1, char)
	ASL_TESTDEFINE( 2, signed char)
	ASL_TESTDEFINE( 3, unsigned char)
	ASL_TESTDEFINE( 4, short)
	ASL_TESTDEFINE( 5, unsigned short)
	ASL_TESTDEFINE( 6, int)
	ASL_TESTDEFINE( 7, unsigned int)
	ASL_TESTDEFINE( 8, long)
	ASL_TESTDEFINE( 9, unsigned long)
	ASL_TESTDEFINE(10, long long)
	ASL_TESTDEFINE(11, unsigned long long)
	ASL_TESTDEFINE2(12, float)
	ASL_TESTDEFINE2(13, double)
	//ASL_TESTDEFINE2(14, long double)
	ASL_TESTDEFINE(15, bool)

#undef ASL_TESTDEFINE2
#undef ASL_TESTDEFINE
}

ASL_UTEST_CASE(serialize, xml_archive_template) {
	std::stringstream ss;
	XmlOutputArchive xout(ss);

	std::string strSrc, strDst;
	strSrc = "Hello world";
	xout & ASL_ARCHIVABLE_NAMEVALUE("str", strSrc);

	std::vector<int> src1, dst1;
	src1.push_back(6454);
	src1.push_back(5345);
	src1.push_back(4564);
	src1.push_back(4364);
	src1.push_back(3252);
	xout & ASL_ARCHIVABLE_NAMEVALUE("list1", src1);

	std::list<int> src2, dst2;
	src2.push_back(6454);
	src2.push_back(5345);
	src2.push_back(4564);
	src2.push_back(4364);
	src2.push_back(3252);
	xout & ASL_ARCHIVABLE_NAMEVALUE("list2", src2);

	std::map<int, int> src3, dst3;
	src3[5] = 6454;
	src3[6] = 5345;
	src3[4] = 4564;
	src3[8] = 4364;
	src3[3] = 3252;
	xout & ASL_ARCHIVABLE_NAMEVALUE("map3", src3);

	xout.Flush();

#ifdef DEBUG_PRINT
	std::cout << ss.str().c_str() << std::endl;
#endif

	XmlInputArchive xin(ss);

	xin & ASL_ARCHIVABLE_NAMEVALUE("str", strDst);
	ASL_UTEST_ASSERT_EQ(strSrc, strDst);
	xin & ASL_ARCHIVABLE_NAMEVALUE("list1", dst1);
	ASL_UTEST_ASSERT_EQ(src1, dst1);
	xin & ASL_ARCHIVABLE_NAMEVALUE("list2", dst2);
	ASL_UTEST_ASSERT_EQ(src2, dst2);
	xin & ASL_ARCHIVABLE_NAMEVALUE("map3", dst3);
	ASL_UTEST_ASSERT_EQ(src3, dst3);
}

ASL_UTEST_CASE(serialize, xml_archive_struct) {
	std::stringstream ss;
	XmlOutputArchive xout(ss);

	People p1, p2;
	p1.strName = "Jim";
	p1.nAge = 18;
	p1.strAddr = "China";
	xout & ASL_ARCHIVABLE_NAMEVALUE("people", p1);

	People2 p3, p4;
	p3.strName = "Jim";
	p3.nAge = 18;
	p3.strAddr = "China";
	xout & ASL_ARCHIVABLE_NAMEVALUE("people2", p3);

	xout.Flush();

#ifdef DEBUG_PRINT
	std::cout << ss.str().c_str() << std::endl;
#endif

	XmlInputArchive xin(ss);

	xin & ASL_ARCHIVABLE_NAMEVALUE("people", p2);
	ASL_UTEST_ASSERT_EQ(p1, p2);
	xin & ASL_ARCHIVABLE_NAMEVALUE("people2", p4);
	ASL_UTEST_ASSERT_EQ(p3, p4);
}

ASL_UTEST_CASE(serialize, json_archive_simple) {
	std::stringstream ss;
	JsonOutputArchive xout(ss);
	srand((unsigned int)time(NULL));

#define ASL_TESTDEFINE(id, type) \
	type src##id = 0, dst##id = 0; \
	for(size_t i = 0; i < sizeof(type); ++i) { \
		((uint8_t*)&src##id)[i] = (uint8_t)rand(); \
		((uint8_t*)&dst##id)[i] = 0; \
	} \
	xout & ASL_ARCHIVABLE_NAMEVALUE("v" #id, src##id); \

#define ASL_TESTDEFINE2(id, type, s, d) \
	type src##id = 0, dst##id = 0; \
	src##id = (type)s; \
	dst##id = d; \
	xout & ASL_ARCHIVABLE_NAMEVALUE("v" #id, src##id); \

	ASL_TESTDEFINE( 1, char)
	ASL_TESTDEFINE( 2, signed char)
	ASL_TESTDEFINE( 3, unsigned char)
	ASL_TESTDEFINE( 4, short)
	ASL_TESTDEFINE( 5, unsigned short)
	ASL_TESTDEFINE( 6, int)
	//ASL_TESTDEFINE( 7, unsigned int)
	//ASL_TESTDEFINE( 8, long)
	//ASL_TESTDEFINE( 9, unsigned long)
	//ASL_TESTDEFINE(10, long long)
	//ASL_TESTDEFINE(11, unsigned long long)
	ASL_TESTDEFINE2(12, float, ASL_PI, 0)
	ASL_TESTDEFINE2(13, double, ASL_PI, 0)
	//ASL_TESTDEFINE2(14, long double, ASL_PI, 0)
	ASL_TESTDEFINE2(15, bool, true, false)

#undef ASL_TESTDEFINE2
#undef ASL_TESTDEFINE

	xout.Flush();

#ifdef DEBUG_PRINT
	std::cout << ss.str().c_str() << std::endl;
#endif

	JsonInputArchive xin(ss);

#define ASL_TESTDEFINE(id, type) \
	xin & ASL_ARCHIVABLE_NAMEVALUE("v" #id, dst##id); \
	ASL_UTEST_ASSERT_EQ(src##id, dst##id); \

#define ASL_TESTDEFINE2(id, type) \
	xin & ASL_ARCHIVABLE_NAMEVALUE("v" #id, dst##id); \
	ASL_UTEST_ASSERT_APPROX_EQ(src##id, dst##id); \
	
	ASL_TESTDEFINE( 1, char)
	ASL_TESTDEFINE( 2, signed char)
	ASL_TESTDEFINE( 3, unsigned char)
	ASL_TESTDEFINE( 4, short)
	ASL_TESTDEFINE( 5, unsigned short)
	ASL_TESTDEFINE( 6, int)
	//ASL_TESTDEFINE( 7, unsigned int)
	//ASL_TESTDEFINE( 8, long)
	//ASL_TESTDEFINE( 9, unsigned long)
	//ASL_TESTDEFINE(10, long long)
	//ASL_TESTDEFINE(11, unsigned long long)
	ASL_TESTDEFINE2(12, float)
	ASL_TESTDEFINE2(13, double)
	//ASL_TESTDEFINE2(14, long double)
	ASL_TESTDEFINE(15, bool)

#undef ASL_TESTDEFINE2
#undef ASL_TESTDEFINE
}

ASL_UTEST_CASE(serialize, json_archive_template) {
	std::stringstream ss;
	JsonOutputArchive xout(ss);

	std::string strSrc, strDst;
	strSrc = "Hello world";
	xout & ASL_ARCHIVABLE_NAMEVALUE("str", strSrc);

	std::vector<int> src1, dst1;
	src1.push_back(6454);
	src1.push_back(5345);
	src1.push_back(4564);
	src1.push_back(4364);
	src1.push_back(3252);
	xout & ASL_ARCHIVABLE_NAMEVALUE("list1", src1);

	std::list<int> src2, dst2;
	src2.push_back(6454);
	src2.push_back(5345);
	src2.push_back(4564);
	src2.push_back(4364);
	src2.push_back(3252);
	xout & ASL_ARCHIVABLE_NAMEVALUE("list2", src2);

	std::map<int, int> src3, dst3;
	src3[5] = 6454;
	src3[6] = 5345;
	src3[4] = 4564;
	src3[8] = 4364;
	src3[3] = 3252;
	xout & ASL_ARCHIVABLE_NAMEVALUE("map3", src3);

	xout.Flush();

#ifdef DEBUG_PRINT
	std::cout << ss.str().c_str() << std::endl;
#endif

	JsonInputArchive xin(ss);

	xin & ASL_ARCHIVABLE_NAMEVALUE("str", strDst);
	ASL_UTEST_ASSERT_EQ(strSrc, strDst);
	xin & ASL_ARCHIVABLE_NAMEVALUE("list1", dst1);
	ASL_UTEST_ASSERT_EQ(src1, dst1);
	xin & ASL_ARCHIVABLE_NAMEVALUE("list2", dst2);
	ASL_UTEST_ASSERT_EQ(src2, dst2);
	xin & ASL_ARCHIVABLE_NAMEVALUE("map3", dst3);
	ASL_UTEST_ASSERT_EQ(src3, dst3);
}

ASL_UTEST_CASE(serialize, json_archive_struct) {
	std::stringstream ss;
	JsonOutputArchive xout(ss);

	People p1, p2;
	p1.strName = "Jim";
	p1.nAge = 18;
	p1.strAddr = "China";
	xout & ASL_ARCHIVABLE_NAMEVALUE("people", p1);

	People2 p3, p4;
	p3.strName = "Jim";
	p3.nAge = 18;
	p3.strAddr = "China";
	xout & ASL_ARCHIVABLE_NAMEVALUE("people2", p3);

	xout.Flush();

#ifdef DEBUG_PRINT
	std::cout << ss.str().c_str() << std::endl;
#endif

	JsonInputArchive xin(ss);

	xin & ASL_ARCHIVABLE_NAMEVALUE("people", p2);
	ASL_UTEST_ASSERT_EQ(p1, p2);
	xin & ASL_ARCHIVABLE_NAMEVALUE("people2", p4);
	ASL_UTEST_ASSERT_EQ(p3, p4);
}
