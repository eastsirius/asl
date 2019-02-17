/**
* @file thread_test.cpp
* @brief 线程工具单元测试
* @author 程行通
*/

#include "asl/utest.hpp"
#include "asl/thread.hpp"

ASL_UTEST_SUITE(thread)

void funThreadPoolTaskProc() {
	asl::Thread::Sleep(10);
}

ASL_UTEST_CASE(thread, thread) {
	asl::ThreadPool tp;
	ASL_UTEST_ASSERT_TRUE(tp.Create(8));
	for(int i = 0; i < 1000; ++i) {
		ASL_UTEST_ASSERT_TRUE(tp.Exec(std::bind(&funThreadPoolTaskProc)));
	}
	tp.Release();
}
