/**
 * @file log.h
 * @brief 日志
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <stdarg.h>
#include <stdint.h>

/* 日志等级 */
#define ASL_LOGLEVEL_ALL    0 /*!< 所有 */

#define ASL_LOGLEVEL_DEBUG  1 /*!< 调试 */
#define ASL_LOGLEVEL_INFO   2 /*!< 信息 */
#define ASL_LOGLEVEL_NOTIFY 3 /*!< 通知 */
#define ASL_LOGLEVEL_WARN   4 /*!< 警告 */
#define ASL_LOGLEVEL_ERROR  5 /*!< 错误 */

#define ASL_LOGLEVEL_NONE   6 /*!< 无 */


#define asl_log_print(level, fmt, ...) \
    asl_log_write(level, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);

#define asl_log_debug(fmt, ...) \
	asl_log_print(ASL_LOGLEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define asl_log_info(fmt, ...) \
	asl_log_print(ASL_LOGLEVEL_INFO, fmt, ##__VA_ARGS__)
#define asl_log_notify(fmt, ...) \
	asl_log_print(ASL_LOGLEVEL_NOTIFY, fmt, ##__VA_ARGS__)
#define asl_log_warn(fmt, ...) \
	asl_log_print(ASL_LOGLEVEL_WARN, fmt, ##__VA_ARGS__)
#define asl_log_error(fmt, ...) \
	asl_log_print(ASL_LOGLEVEL_ERROR, fmt, ##__VA_ARGS__)


#define ASL_LOG_MAX_OUTPUT_NUM 16
#define ASL_LOG_MAX_LENGTH (16 * 1024)


namespace ASL_NAMESPACE {
	/**
	 * @brief 日志输出类型
	 */
	typedef enum _asl_log_output_type_t {
		ALOT_STDOUT,    /*!< 标准输出 */
		ALOT_FILE,      /*!< 文件输出 */
		ALOT_CUSTOM     /*!< 自定义输出 */
	} asl_log_output_type_t;

	/**
	 * @brief 日志输出函数
	 * @param level 日志等级
	 * @param log 日志
	 */
	typedef void (*asl_log_output_proc_t)(int level, const char* log);

    /**
     * @brief 日志格式化函数
     * @param buf 输出缓存
     * @param size 输出缓存大小
     * @param msg 日志消息
     * @param args 格式化参数
     */
	typedef void (*asl_log_formatter_t)(char* buf, int size, const struct _asl_log_msg_t* msg, va_list args);

    /**
     * @brief 日志文件名生成函数
     * @param buf 输出缓存
     * @param size 输出缓存大小
     */
    typedef void (*asl_log_file_namer_t)(char* buf, int size);

	/**
	 * @brief 日志输出配置
	 */
	typedef struct _asl_log_output_t {
		asl_log_output_type_t type;         /*!< 日志输出类型 */
		char path[1024];                    /*!< 输出路径(仅文件输出) */
		asl_log_output_proc_t output_proc;  /*!< 自定义输出函数(仅自定义输出) */
		int level;                      	/*!< 日志等级 */
	} asl_log_output_t;

    /**
     * @brief 日志消息
     */
    typedef struct _asl_log_msg_t {
        uint64_t timestamp;                 /*!< 日志时间戳 */
        int level;                      	/*!< 日志等级 */
        const char* file;                   /*!< 代码文件 */
        const char* func;                   /*!< 代码所在函数 */
        int line;                           /*!< 代码所在行 */
        const char* format;                 /*!< 格式化字符串 */
    } asl_log_msg_t;


	/**
	 * @brief 初始化日志
	 * @return 成功返回0，失败返回负数
	 */
	int asl_log_init();

	/**
	 * @brief 清理日志
	 */
	void asl_log_release();

	/**
	 * @brief 配置日志
	 * @param outputs 输出配置列表
	 * @param output_num 输出配置数
	 */
	void asl_log_config(const asl_log_output_t* outputs, int output_num);

	/**
	 * @brief 打印日志
	 * @param level 日志等级
	 * @param file 代码文件
	 * @param func 代码函数
	 * @param line 代码行
	 * @param fmt 格式化文本
	 */
	void asl_log_write(int level, const char* file, const char* func, int line, const char* fmt, ...);

	/**
	 * @brief 打印日志
	 * @param level 日志等级
	 * @param file 代码文件
	 * @param func 代码函数
	 * @param line 代码行
	 * @param fmt 格式化文本
	 * @param args 参数列表
	 */
	void asl_log_write2(int level, const char* file, const char* func, int line, const char* fmt, va_list args);

    /**
     * @brief 设置日志格式化函数
     * @param formatter 日志格式化函数
     */
	void asl_log_set_formatter(asl_log_formatter_t formatter);

    /**
     * @brief 日志文件名生成函数
     * @param namer 日志文件名生成函数
     */
    void asl_log_set_file_namer(asl_log_file_namer_t namer);
}
