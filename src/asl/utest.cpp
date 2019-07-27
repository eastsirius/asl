/**
 * @file utest.cpp
 * @brief 单元测试工具
 * @author 程行通
 */

#include "utest.hpp"
#include "utils.hpp"
#include "time.hpp"
#include <stdio.h>
#include <stdarg.h>
#ifdef WINDOWS
#  include <windows.h>
#endif

#define ASL_UTEST_INC_ASSERT(exp, msg, ...) \
    if(!(exp)) { printf(msg, ##__VA_ARGS__); abort(); }

#define ASL_UTEST_RESULT_RUN        " RUN      ", false
#define ASL_UTEST_RESULT_PASSED     "  PASSED  ", false
#define ASL_UTEST_RESULT_OK         "       OK ", false
#define ASL_UTEST_RESULT_FAILED     "  FAILED  ", true
#define ASL_UTEST_RESULT_SEPARATOR  "----------", false
#define ASL_UTEST_RESULT_SEPARATOR2 "==========", false

namespace ASL_NAMESPACE {
    bool UTestCase::s_bLastRunFailed = false;

    UTestCase::UTestCase(const char* suite, const char* name,
        FuncUTestCaseTestProc test_proc)
        : m_strName(name), m_strSuite(suite), 
        m_funcTestProc(test_proc), m_bRunFailed(false) {
    }

    void UTestCase::RunTest() {
        UTest::CommandPrint(ASL_UTEST_RESULT_RUN, "%s.%s\n",
            m_strSuite.c_str(), m_strName.c_str());

		int64_t begin_time = asl_get_ms_time();
        try {
            s_bLastRunFailed = false;
            m_funcTestProc();
        } catch(...) {
        }
        int64_t end_time = asl_get_ms_time();
        m_bRunFailed = s_bLastRunFailed;

        if(m_bRunFailed) {
            UTest::CommandPrint(ASL_UTEST_RESULT_FAILED, "%s.%s (%d ms)\n",
                m_strSuite.c_str(), m_strName.c_str(), (int)(end_time - begin_time));
        } else {
            UTest::CommandPrint(ASL_UTEST_RESULT_OK, "%s.%s (%d ms)\n",
                m_strSuite.c_str(), m_strName.c_str(), (int)(end_time - begin_time));
        }
    }


    UTestSuite::UTestSuite(const char* name) : m_bCreated(false), m_strName(name) {
    }

    void UTestSuite::Create() {
        m_bCreated = true;
    }
    
    void UTestSuite::RunTest() {
        UTest::CommandPrint(ASL_UTEST_RESULT_SEPARATOR, "%d test cases from %s\n",
            (int)m_lstTestCases.size(), m_strName.c_str());

        int64_t begin_time = asl_get_ms_time();
        for(size_t i = 0; i < m_lstTestCases.size(); ++i) {
            m_lstTestCases[i].RunTest();
        }
        int64_t end_time = asl_get_ms_time();

        UTest::CommandPrint(ASL_UTEST_RESULT_SEPARATOR, "%d test cases from %s (%d ms total)\n\n",
            (int)m_lstTestCases.size(), m_strName.c_str(), (int)(end_time - begin_time));
    }
    
    void UTestSuite::RegisterCase(const char* name, FuncUTestCaseTestProc test_proc) {
        for(size_t i = 0; i < m_lstTestCases.size(); ++i) {
            ASL_UTEST_INC_ASSERT(m_lstTestCases[i].GetName() != name,
                "repeat test case named \"%s\" in suite \"%s\"",
                name, m_strName.c_str());
        }
        m_lstTestCases.push_back(UTestCase(m_strName.c_str(), name, test_proc));
    }

    void UTestSuite::GetFailedList(std::vector<std::string>& cases) {
        for(size_t i = 0; i < m_lstTestCases.size(); ++i) {
            if(m_lstTestCases[i].GetResultFailed()) {
                cases.push_back(m_strName + "." + m_lstTestCases[i].GetName());
            }
        }
    }


    UTest::UTest() {
    }

    void UTest::RunTest() {
        int case_num = 0, suite_num = 0;
        for(size_t i = 0; i < m_lstTestSuites.size(); ++i) {
            if(m_lstTestSuites[i].IsCreated()) {
                ++suite_num;
                case_num += m_lstTestSuites[i].GetCaseNum();
            }
        }
        UTest::CommandPrint(ASL_UTEST_RESULT_SEPARATOR2, "Running %d test cases from %d test suites.\n\n",
            case_num, suite_num);

        int64_t begin_time = asl_get_ms_time();
        for(size_t i = 0; i < m_lstTestSuites.size(); ++i) {
            if(m_lstTestSuites[i].IsCreated()) {
                m_lstTestSuites[i].RunTest();
            }
        }
        int64_t end_time = asl_get_ms_time();

        std::vector<std::string> failed_cases;
        for(size_t i = 0; i < m_lstTestSuites.size(); ++i) {
            if(m_lstTestSuites[i].IsCreated()) {
                m_lstTestSuites[i].GetFailedList(failed_cases);
            }
        }

        UTest::CommandPrint(ASL_UTEST_RESULT_SEPARATOR2, "%d test cases from %d test suites ran. (%d ms total)\n",
            case_num, suite_num, (int)(end_time - begin_time));
        UTest::CommandPrint(ASL_UTEST_RESULT_PASSED, "%d test cases.\n", (int)(case_num - failed_cases.size()));
        if(failed_cases.size() > 0) {
            UTest::CommandPrint(ASL_UTEST_RESULT_FAILED, "%d test case, listed below:\n", (int)failed_cases.size());
            for(size_t i = 0; i < failed_cases.size(); ++i) {
                UTest::CommandPrint(ASL_UTEST_RESULT_FAILED, "%s\n", failed_cases[i].c_str());
            }
        }
        printf("\n");

#ifdef WINDOWS
        system("pause");
#endif
    }
    
    void UTest::DeclareTestSuite(const char* name) {
        for(size_t i = 0; i < m_lstTestSuites.size(); ++i) {
            if(m_lstTestSuites[i].GetName() == name) {
                return;
            }
        }
        m_lstTestSuites.push_back(UTestSuite(name));
    }

    void UTest::RegisterTestSuite(const char* name) {
        UTestSuite* suite = NULL;
        for(size_t i = 0; i < m_lstTestSuites.size(); ++i) {
            ASL_UTEST_INC_ASSERT(m_lstTestSuites[i].GetName() != name
                || !m_lstTestSuites[i].IsCreated(),
                "repeat test suite named \"%s\"", name);

            if(m_lstTestSuites[i].GetName() == name) {
                suite = &m_lstTestSuites[i];
                break;
            }
        }

        if(!suite) {
            m_lstTestSuites.push_back(UTestSuite(name));
            suite = &m_lstTestSuites.back();
        }

        suite->Create();
    }
    
    void UTest::RegisterTestCase(const char* suite, const char* name,
        FuncUTestCaseTestProc test_proc) {
        int id = -1;
        for(size_t i = 0; i < m_lstTestSuites.size(); ++i) {
            if(m_lstTestSuites[i].GetName() == suite
                && m_lstTestSuites[i].IsCreated()) {
                id = (int)i;
                break;
            }
        }
        
        ASL_UTEST_INC_ASSERT(id >= 0, "register test case \"%s\" failed,"
            " unknown test suite \"%s\"", name, suite);
        m_lstTestSuites[id].RegisterCase(name, test_proc);
    }

    UTest& UTest::Instance() {
        static UTest instance;
        return instance;
    }

    void UTest::CommandPrint(const char* result, bool error, const char* fmt, ...) {
#ifdef WINDOWS
        if(error) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        }
        printf("[%s] ", result);

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
#else
        if(error) {
            printf("\033[40;31m [%s]  \033[0m", result);
        } else {
            printf("\033[40;32m [%s]  \033[0m", result);
        }

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
#endif
    }
}
