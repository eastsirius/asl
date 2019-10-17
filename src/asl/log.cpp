/**
 * @file log.cpp
 * @brief 日志
 * @author 程行通
 */

#include "log.hpp"
#include "utils.hpp"
#include "thread.hpp"
#include "time.hpp"
#include <fstream>
#include <cstring>

namespace ASL_NAMESPACE {
    static int g_output_num = 0;
    static asl_log_output_t g_outputs[ASL_LOG_MAX_OUTPUT_NUM];
    static int g_module_min_level = 0;
    static Mutex g_loggers_lock;
    static asl_log_formatter_t g_formatter = NULL;
    static asl_log_file_namer_t g_file_namer = NULL;

    static const char* g_szLogLevelStrings[] = {
            "all",
            "debug",
            "info",
            "notify",
            "warning",
            "error",
            "none"
    };


    /**
     * @brief 标准输出
     */
    static void _asl_log_std_output(asl_log_output_t* output, int level, const char* log);

    /**
     * @brief 文件输出
     */
    static void _asl_log_file_output(asl_log_output_t* output, int level, const char* log);

    /**
     * @brief 自定义输出
     */
    static void _asl_log_custom_output(asl_log_output_t* output, int level, const char* log);

    /**
     * @brief 日志格式化函数
     */
    static void _asl_log_formatter(char* buf, int size, const asl_log_msg_t* msg, va_list args);

    /**
     * @brief 日志文件名生成函数
     */
    static void _asl_log_file_namer(char* buf, int size);


    int asl_log_init() {
        g_output_num = 0;
        memset(g_outputs, 0, sizeof(g_outputs));
        g_module_min_level = 0;

        asl_log_set_formatter(NULL);
        asl_log_set_file_namer(NULL);

        asl_log_output_t opt;
        memset(&opt, 0, sizeof(opt));
        opt.type = ALOT_STDOUT;
        opt.level = ASL_LOGLEVEL_ALL;
        asl_log_config(&opt, 1);

        return 0;
    }

    void asl_log_release() {
    }

    void asl_log_config(const asl_log_output_t* outputs, int output_num) {
        AutoLocker<Mutex> lock(g_loggers_lock);

        g_output_num = ASL_MIN(output_num, ASL_LOG_MAX_OUTPUT_NUM);
        memcpy(g_outputs, outputs, g_output_num * sizeof(asl_log_output_t));

        g_module_min_level = ASL_LOGLEVEL_NONE;
        for(int i = 0; i < g_output_num; ++i) {
            if(g_outputs[i].level < g_module_min_level) {
                g_module_min_level = g_outputs[i].level;
            }
        }
    }

    void asl_log_write(int level, const char* file, const char* func, int line, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        asl_log_write2(level, file, func, line, fmt, args);
        va_end(args);
    }

    void asl_log_write2(int level, const char* file, const char* func, int line, const char* fmt, va_list args) {
        const char* p1, *p2, *p;

        if(level < g_module_min_level) {
            return;
        }

        p1 = strrchr(file, '/') + 1;
        p2 = strrchr(file, '\\') + 1;
        p = ASL_MAX(p1, p2);
        p = ASL_MAX(p, file);

        do {
            asl_log_msg_t msg = {0};
            char acBuffer[ASL_LOG_MAX_LENGTH];
            int i;

            msg.timestamp = asl_get_ms_time();
            msg.level = level;
            msg.file = p;
            msg.func = func;
            msg.line = line;
            msg.format = fmt;
            g_formatter(acBuffer, ASL_LOG_MAX_LENGTH - 2, &msg, args);
            strcat(acBuffer, "\n");

            g_loggers_lock.Lock();
            for(i = 0; i < g_output_num; ++i) {
                int level_cfg = g_outputs[i].level;
                if(level < level_cfg) {
                    continue;
                }

                switch(g_outputs[i].type) {
                    case ALOT_STDOUT:
                        _asl_log_std_output(&g_outputs[i], level, acBuffer);
                        break;
                    case ALOT_FILE:
                        _asl_log_file_output(&g_outputs[i], level, acBuffer);
                        break;
                    case ALOT_CUSTOM:
                        _asl_log_custom_output(&g_outputs[i], level, acBuffer);
                        break;
                    default:
                        break;
                }
            }
            g_loggers_lock.Unlock();
        } while(false);
    }

    void asl_log_set_formatter(asl_log_formatter_t formatter) {
        g_formatter = formatter ? formatter : _asl_log_formatter;
    }

    void asl_log_set_file_namer(asl_log_file_namer_t namer) {
        g_file_namer = namer ? namer : _asl_log_file_namer;
    }


    static void _asl_log_std_output(asl_log_output_t* output, int level, const char* log) {
        printf("%s", log);
    }

    static void _asl_log_file_output(asl_log_output_t* output, int level, const char* log) {
        char acPath[1024];

        strcpy(acPath, output->path);
        strcat(acPath, "/");
        int len = strlen(acPath);
        g_file_namer(acPath + len, sizeof(acPath) - len);

        std::ofstream fout(acPath, std::ofstream::app | std::ofstream::binary);
        if(!fout) {
            return;
        }
        fout.write(log, strlen(log));
        fout.close();
    }

    static void _asl_log_custom_output(asl_log_output_t* output, int level, const char* log){
        if(output->output_proc) {
            output->output_proc(level, log);
        }
    }

    static void _asl_log_formatter(char* buf, int size, const asl_log_msg_t* msg, va_list args) {
        Datetime dt = Datetime::ToLocalTime(Time(msg->timestamp * 1000));
        snprintf(buf, size - 1, "%2.2d:%2.2d:%2.2d.%3.3d [%s] %s:%d ", dt.GetHour(), dt.GetMinute(),
                dt.GetSecond(), dt.GetMillisecond(), g_szLogLevelStrings[msg->level], msg->file, msg->line);
        int len = (int)strlen(buf);
        vsnprintf(buf + len, size - 1 - len, msg->format, args);
    }

    static void _asl_log_file_namer(char* buf, int size) {
        Datetime dt = Datetime::GetLocalTime();
        snprintf(buf, size, "%4.4d-%2.2d-%2.2d.txt", dt.GetYear(), dt.GetMonth(), dt.GetDay());
    }
}
