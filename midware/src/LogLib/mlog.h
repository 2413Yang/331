#ifndef base_xlogbase_H_
#define base_xlogbase_H_
#include <stdio.h>
#include <string.h>
#include <string>

#define MLOG_LEVEL_SHT 0
#define MLOG_LEVEL_CRI 1
#define MLOG_LEVEL_ERR 2
#define MLOG_LEVEL_WAR 3
#define MLOG_LEVEL_NOT 4
#define MLOG_LEVEL_INF 5
#define MLOG_LEVEL_DBG 6

typedef enum
{
	LOG_SHT = MLOG_LEVEL_SHT, /* Shut down the system NOW*/
	LOG_CRI = MLOG_LEVEL_CRI, /*Unexpected unrecoverable error */
	LOG_ERR = MLOG_LEVEL_ERR, /*Unexpected recoverable error */
	LOG_WAR = MLOG_LEVEL_WAR, /*Expected error */
	LOG_NOT = MLOG_LEVEL_NOT, /*Warnings */
	LOG_INF = MLOG_LEVEL_INF, /*Information*/
	LOG_DBG = MLOG_LEVEL_DBG, /*Debug messages*/
	LOG_LAST,
} LOG_LEV_EN;

#ifndef MLOG_LEVEL
#define MLOG_LEVEL MLOG_LEVEL_DBG 
#endif
/***************************
 *  函数 log 打印函数
 *  函数参数：Level：Level 打印级别，见LOG_LEV_EN结构体说明
 *                FileName：输出日志信息的源码文件名
 *                FuncName：输出日志信息的函数名称
 *                LineNumber：输出日志信息在源码中的行号
 *                fmt：打印信息，为可变参数，最大长度为4096 字节，超出返回失败。
 *  函数返回：大于0，为成功，小于等于0为失败。
 *
 *  说明：qnx系统打印log信息.
 *          输出内容：开机到运行时刻时间（毫秒）, 日志信息等级数值,模块ID(主要、次要),模块名称,文件名,函数名称,行号,日志信息。
 *          获取hmi数据示例:sloginfo -m 60000,那么只显示和打印hmi的数据。
 ***************************/
int xLog(LOG_LEV_EN emLevel, const char *pFileName, const char *pFuncName,
		 int iLineNumber, const char *fmt, ...);

extern void setPrintLevel(LOG_LEV_EN enlevel);
#define SLOG
// #define CONSOLE

#if defined(SLOG)
    #if MLOG_LEVEL >= MLOG_LEVEL_ERR
        #define LOGERR(fmt, ...) xLog(LOG_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #else
        #define LOGERR(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_WAR
        #define LOGWAR(fmt, ...) xLog(LOG_WAR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #else
        #define LOGWAR(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_NOT
        #define LOGNOT(fmt, ...) xLog(LOG_NOT, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #else
        #define LOGNOT(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_INF
        #define LOGINF(fmt, ...) xLog(LOG_INF, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #else
        #define LOGINF(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_DBG
        #define LOGDBG(fmt, ...) xLog(LOG_DBG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #else
        #define LOGDBG(fmt, ...) do{}while(0)
    #endif

#elif defined(CONSOLE)
    #if MLOG_LEVEL >= MLOG_LEVEL_ERR
        #define LOGERR(fmt, ...) printf("[ERR][%s][%s][%s][%d]" fmt "\n", MODULENAME, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
    #else
        #define LOGERR(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_WAR
        #define LOGWAR(fmt, ...) printf("[WAR][%s][%s][%s][%d]" fmt "\n", MODULENAME, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
    #else
        #define LOGWAR(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_NOT
        #define LOGNOT(fmt, ...) printf("[NOT][%s][%s][%s][%d]" fmt "\n", MODULENAME, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
    #else
        #define LOGNOT(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_INF
        #define LOGINF(fmt, ...) printf("[INF][%s][%s][%s][%d]" fmt "\n", MODULENAME, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
    #else
        #define LOGINF(fmt, ...) do{}while(0)
    #endif
    #if MLOG_LEVEL >= MLOG_LEVEL_DBG
        #define LOGDBG(fmt, ...) printf("[DBG][%s][%s][%s][%d]" fmt "\n", MODULENAME, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
    #else
        #define LOGDBG(fmt, ...) do{}while(0)
    #endif
#else
#define LOGERR(fmt, ...)
#define LOGWAR(fmt, ...)
#define LOGINF(fmt, ...)
#define LOGDBG(fmt, ...)
#define LOGNOT(fmt, ...)
#endif

#if MLOG_LEVEL >= MLOG_LEVEL_INF
    #define LOGHEX(buf, length, param)                                \
        {                                                             \
            int len = length > 100 ? 100 :length;/*太长就截断打印*/    \
            char tmpBuf[(len)*3 + 50];                                \
            memset(tmpBuf, 0, (len)*3 + 50);                          \
            for (unsigned int i = 0; i < (unsigned int)(len); i++)    \
                sprintf(&tmpBuf[i * 3], "%02X ", buf[i]);             \
            LOGINF("HEX [%s]: %s ", param, tmpBuf);                   \
        }

    #define PRFHEX(buf, length, param)                                \
        {                                                             \
            int len = length > 100 ? 100 :length;/*太长就截断打印*/    \
            char tmpBuf[(len)*3 + 50];                                \
            memset(tmpBuf, 0, (len)*3 + 50);                          \
            for (unsigned int i = 0; i < (unsigned int)(len); i++)    \
                sprintf(&tmpBuf[i * 3], "%02X ", buf[i]);             \
            printf("HEX [%s]: %s \n", param, tmpBuf);                 \
        }
    // printf("HEX [%s]: %s \n\n", param, tmpBuf);
#else
    #define LOGHEX(buf, length, param) do{}while(0)
    #define PRFHEX(buf, length, param) do{}while(0)
#endif

std::string getModuleID(int pid = 0);

#endif
