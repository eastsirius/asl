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


    int asl_log_init() {
        g_output_num = 0;
        memset(g_outputs, 0, sizeof(g_outputs));
        g_module_min_level = 0;

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

    void asl_log_write(int level, const char* file, int line, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        asl_log_write2(level, file, line, fmt, args);
        va_end(args);
    }

    void asl_log_write2(int level, const char* file, int line, const char* fmt, va_list args) {
        const char* p1, *p2, *p;

        if(level < g_module_min_level) {
            return;
        }

        p1 = strrchr(file, '/') + 1;
        p2 = strrchr(file, '\\') + 1;
        p = ASL_MAX(p1, p2);
        p = ASL_MAX(p, file);

        do {
            char acBuffer[ASL_LOG_MAX_LENGTH];
            Datetime dt = Datetime::GetLocalTime();
            int len, i;

            /* 格式化日志 */
            sprintf(acBuffer, "%2.2d:%2.2d:%2.2d.%3.3d %s: %s(%d): ",
                    dt.GetHour(), dt.GetMinute(), dt.GetSecond(), dt.GetMillisecond(),
                    g_szLogLevelStrings[level], p, line);
            len = strlen(acBuffer);
            vsprintf(acBuffer + len, fmt, args);
            len = strlen(acBuffer);
            strcpy(acBuffer + len, "\n");
            len = strlen(acBuffer);

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


    static void _asl_log_std_output(asl_log_output_t* output, int level, const char* log) {
        printf("%s", log);
    }

    static void _asl_log_file_output(asl_log_output_t* output, int level, const char* log) {
        char acPath[1024];
        Datetime dt = Datetime::GetLocalTime();

        sprintf(acPath, "%s/%4.4d-%2.2d-%2.2d.txt", output->path, dt.GetYear(), dt.GetMonth(), dt.GetDay());

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
}
