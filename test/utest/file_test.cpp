/**
* @file file_test.cpp
* @brief 文件工具单元测试
* @author 程行通
*/

#include "asl/utest.hpp"
#include "asl/file.hpp"

using namespace ASL_NAMESPACE;

ASL_UTEST_SUITE(file)

ASL_UTEST_CASE(file, file_test) {
	const char* szFilename = "aslutest.bin";
	const int nBufferSize = 1024;
	char acBuffer1[nBufferSize] = {0};
	char acBuffer2[nBufferSize] = {0};

	File file;
	ASL_UTEST_ASSERT_TRUE(file.Open(szFilename, File::OF_Out));
	ASL_UTEST_ASSERT_EQ(file.Write(acBuffer1, nBufferSize), nBufferSize);
	ASL_UTEST_ASSERT_EQ((int)file.GetFileSize(), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), nBufferSize);
	file.Close();

	ASL_UTEST_ASSERT_TRUE(file.Open(szFilename, File::OF_In));
	ASL_UTEST_ASSERT_EQ(file.GetPos(), 0);
	ASL_UTEST_ASSERT_EQ(file.GetFileSize(), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), nBufferSize);
	ASL_UTEST_ASSERT_EQ(memcmp(acBuffer1, acBuffer2, nBufferSize), 0);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), 0);
	file.Close();

	ASL_UTEST_ASSERT_TRUE(file.Open(szFilename, File::OF_Out | File::OF_App));
	ASL_UTEST_ASSERT_EQ(file.GetFileSize(), nBufferSize);
	//ASL_UTEST_ASSERT_EQ(file.GetPos(), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Write(acBuffer1, nBufferSize), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.GetFileSize(), 2 * nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), 2 * nBufferSize);
	file.Close();

	ASL_UTEST_ASSERT_TRUE(file.Open(szFilename, File::OF_In));
	ASL_UTEST_ASSERT_EQ(file.GetPos(), 0);
	ASL_UTEST_ASSERT_EQ(file.GetFileSize(), 2 * nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), nBufferSize);
	ASL_UTEST_ASSERT_EQ(memcmp(acBuffer1, acBuffer2, nBufferSize), 0);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), nBufferSize);
	ASL_UTEST_ASSERT_EQ(memcmp(acBuffer1, acBuffer2, nBufferSize), 0);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), 2 * nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), 0);
	file.Close();

	ASL_UTEST_ASSERT_TRUE(file.Open(szFilename, File::OF_In));
	ASL_UTEST_ASSERT_EQ(file.GetPos(), 0);
	ASL_UTEST_ASSERT_EQ(file.GetFileSize(), 2 * nBufferSize);
	file.Seek(nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), nBufferSize);
	ASL_UTEST_ASSERT_EQ(memcmp(acBuffer1, acBuffer2, nBufferSize), 0);
	ASL_UTEST_ASSERT_EQ(file.GetPos(), 2 * nBufferSize);
	ASL_UTEST_ASSERT_EQ(file.Read(acBuffer2, nBufferSize), 0);
	file.Close();

	ASL_UTEST_ASSERT_TRUE(File::RemoveFile(szFilename));
}
