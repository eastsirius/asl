/**
 * @file log.h
 * @brief 日志
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include <stdarg.h>

/* 日志等级 */
#define ASL_LOGLEVEL_ALL    0 /*!< 所有 */

#define ASL_LOGLEVEL_DEBUG  1 /*!< 调试 */
#define ASL_LOGLEVEL_INFO   2 /*!< 信息 */
#define ASL_LOGLEVEL_NOTIFY 3 /*!< 通知 */
#define ASL_LOGLEVEL_WARN   4 /*!< 警告 */
#define ASL_LOGLEVEL_ERROR  5 /*!< 错误 */

#define ASL_LOGLEVEL_NONE   6 /*!< 无 */


#define asl_log_print(level, fmt, ...) \
    asl_log_write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__);

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
	 * @brief 日志输出配置
	 */
	typedef struct _asl_log_output_t {
		asl_log_output_type_t type;         /*!< 日志输出类型 */
		char path[1024];                    /*!< 输出路径(仅文件输出) */
		asl_log_output_proc_t output_proc;  /*!< 自定义输出函数(仅自定义输出) */
		int level;                      	/*!< 日志等级 */
	} asl_log_output_t;


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
	 * @param line 代码行
	 * @param fmt 格式化文本
	 */
	void asl_log_write(int level, const char* file, int line, const char* fmt, ...);

	/**
	 * @brief 打印日志
	 * @param level 日志等级
	 * @param file 代码文件
	 * @param line 代码行
	 * @param fmt 格式化文本
	 * @param args 参数列表
	 */
	void asl_log_write2(int level, const char* file, int line, const char* fmt, va_list args);
}
