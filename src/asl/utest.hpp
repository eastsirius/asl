/**
 * @file utest.hpp
 * @brief 单元测试工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <string>
#include <vector>
#include <exception>

#ifdef MSVC
#  pragma warning(push)
#  pragma warning(disable:4251)
#endif

#define ASL_UTEST_RUN_TEST() \
    ASL_NAMESPACE::UTest::Instance().RunTest()

#define ASL_UTEST_SUITE(name) \
    ASL_NAMESPACE::UTestSuiteRegister g_asl_utest_suite_register_##name(#name); \

#define ASL_UTEST_CASE(suite, name) \
    static void __asl_utest_case_test_proc_##suite##_##name(); \
    ASL_NAMESPACE::UTestCaseRegister g_asl_utest_case_register_##suite##_##name( \
        #suite, #name, __asl_utest_case_test_proc_##suite##_##name); \
    static void __asl_utest_case_test_proc_##suite##_##name() \


#define ASL_UTEST_ASSERT(exp, fmt, ...) \
    if(!(exp)) { \
        ASL_NAMESPACE::UTestCase::SetRunFailed(); \
        printf("%s(%d): error: Value of: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
        throw std::exception(); \
    } \

#define ASL_UTEST_ASSERT_TRUE(exp) \
    do { \
        auto r = (exp); \
        ASL_UTEST_ASSERT(r, "%s\n  value:%s\n", #exp, "false"); \
    } while(false); \

#define ASL_UTEST_ASSERT_FALSE(exp) \
    do { \
        auto r = !(exp); \
        ASL_UTEST_ASSERT(r, "%s\n  value:%s\n", #exp, "true"); \
    } while(false); \
    
#define ASL_UTEST_ASSERT_EQ(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 == r2, "%s\n", #v1 " == " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_NE(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 != r2, "%s\n", #v1 " != " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_APPROX_EQ(v1, v2) \
    do { \
        double r1 = (v1); \
        double r2 = (v2); \
        double r3 = r1 - r2; \
        ASL_UTEST_ASSERT(r3 > -0.0001 && r3 < 0.0001, "%s\n", #v1 " == " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_APPROX_NE(v1, v2) \
    do { \
        double r1 = (v1); \
        double r2 = (v2); \
        double r3 = r1 - r2; \
        ASL_UTEST_ASSERT(r3 < -0.0001 || r3 > 0.0001, "%s\n", #v1 " != " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_LE(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 <= r2, "%s\n", #v1 " <= " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_LT(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 < r2, "%s\n", #v1 " < " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_GE(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 >= r2, "%s\n", #v1 " >= " #v2); \
    } while(false); \

#define ASL_UTEST_ASSERT_GT(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 > r2, "%s\n", #v1 " > " #v2); \
    } while(false); \


#define ASL_UTEST_EXPECT(exp, fmt, ...) \
    if(!(exp)) { \
        ASL_NAMESPACE::UTestCase::SetRunFailed(); \
        printf("%s(%d): error: Value of: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \

#define ASL_UTEST_EXPECT_TRUE(exp) \
    do { \
        auto r = (exp); \
        ASL_UTEST_EXPECT(r, "%s\n  value:%s\n", #exp, "false"); \
    } while(false); \

#define ASL_UTEST_EXPECT_EQ(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 == r2, "%s\n", #v1 " == " #v2); \
    } while(false); \

#define ASL_UTEST_EXPECT_NE(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 != r2, "%s\n", #v1 " != " #v2); \
    } while(false); \

#define ASL_UTEST_EXPECT_LE(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 <= r2, "%s\n", #v1 " <= " #v2); \
    } while(false); \

#define ASL_UTEST_EXPECT_LT(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 < r2, "%s\n", #v1 " < " #v2); \
    } while(false); \

#define ASL_UTEST_EXPECT_GE(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 >= r2, "%s\n", #v1 " >= " #v2); \
    } while(false); \

#define ASL_UTEST_EXPECT_GT(v1, v2) \
    do { \
        auto r1 = (v1); \
        auto r2 = (v2); \
        ASL_UTEST_ASSERT(r1 > r2, "%s\n", #v1 ">" #v2); \
    } while(false); \


namespace ASL_NAMESPACE {
    /**
     * @brief 测试用例测试函数类型
     */
    typedef void (*FuncUTestCaseTestProc)();

    /**
     * @brief 测试用例
     */
    class UTestCase {
    private:
        friend class UTestSuite;

        UTestCase(const char* suite, const char* name,
            FuncUTestCaseTestProc test_proc);

    public:
        /**
         * @brief 运行测试
         */
        void RunTest();

        /**
         * @brief 获取测试套件名
         * @return 返回测试套件名
         */
        const std::string& GetName() const {
            return m_strName;
        }

        /**
         * @brief 获取测试结果是否为失败
         * @return 返回测试结果是否为失败
         */
        bool GetResultFailed() const {
            return m_bRunFailed;
        }

    public:
        /**
         * @brief 设置错误
         */
        static void SetRunFailed() {
            s_bLastRunFailed = true;
        }

    private:
        std::string m_strName;  /*!< 测试用例名 */
        std::string m_strSuite; /*!< 测试套件名 */
        FuncUTestCaseTestProc m_funcTestProc; /*!< 测试函数 */
        bool m_bRunFailed;      /*!< 测试出错 */
        static bool s_bLastRunFailed;   /*!< 最后一次测试结果 */
    };

    /**
     * @brief 测试套件
     */
    class UTestSuite {
    private:
        friend class UTest;

        UTestSuite(const char* name);

    public:
        /**
         * @brief 创建套件
         */
        void Create();

        /**
         * @brief 运行测试
         */
        void RunTest();

        /**
         * @brief 注册测试套件
         * @param name 测试用例名
         * @param test_proc 测试函数
         */
        void RegisterCase(const char* name, FuncUTestCaseTestProc test_proc);

        /**
         * @brief 获取测试套件名
         * @return 返回测试套件名
         */
        const std::string& GetName() const {
            return m_strName;
        }

        /**
         * @brief 获取测试套件是否已创建
         * @return 返回测试套件是否已创建
         */
        bool IsCreated() const {
            return m_bCreated;
        }

        /**
         * @brief 获取测试用例数
         * @return 返回测试用例数
         */
        int GetCaseNum() const {
            return m_lstTestCases.size();
        }

        /**
         * @brief 获取失败列表
         */
        void GetFailedList(std::vector<std::string>& cases);

    private:
        bool m_bCreated;        /*!< 是否已创建 */
        std::string m_strName;  /*!< 测试套件名 */
        std::vector<UTestCase> m_lstTestCases; /*!< 测试用例列表 */
    };

    /**
     * @brief 单元测试
     */
    class UTest {
    private:
        UTest();

    public:
        /**
         * @brief 运行测试
         */
        void RunTest();
        
        /**
         * @brief 声明测试套件
         * @param name 测试套件名
         */
        void DeclareTestSuite(const char* name);

        /**
         * @brief 注册测试套件
         * @param name 测试套件名
         */
        void RegisterTestSuite(const char* name);

        /**
         * @brief 注册测试套件
         * @param suite 测试套件名
         * @param name 测试用例名
         * @param test_proc 测试函数
         */
        void RegisterTestCase(const char* suite, const char* name,
            FuncUTestCaseTestProc test_proc);

    public:
        /**
         * @brief 唯一实例
         * @return 返回唯一实例引用
         */
        static UTest& Instance();

        /**
         * @brief 命令行打印
         * @param result 执行结果
         * @param error 是否为错误信息
         * @param fmt 格式字符串
         */
        static void CommandPrint(const char* result, bool error, const char* fmt, ...);

    private:
        std::vector<UTestSuite> m_lstTestSuites; /*!< 测试套件列表 */
    };

    /**
     * @brief 测试用例注册器
     */
    class UTestCaseRegister {
    public:
        UTestCaseRegister(const char* suite, const char* name,
            FuncUTestCaseTestProc test_proc) {
            UTest::Instance().RegisterTestCase(suite, name, test_proc);
        }
    };

    /**
     * @brief 测试套件注册器
     */
    class UTestSuiteRegister {
    public:
        UTestSuiteRegister(const char* name) {
            UTest::Instance().RegisterTestSuite(name);
        }
    };
}

#ifdef MSVC
#  pragma warning(pop)
#endif
