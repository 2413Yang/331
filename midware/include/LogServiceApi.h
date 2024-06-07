#ifndef _LOGSERVICEAPI_H_
#define _LOGSERVICEAPI_H_
#include "IPC/IPCCore.h"
#include <string>

namespace ZH
{
namespace logService
{
    enum EnLogLevel
    {
        LOGLEVEL_ERR,
        LOGLEVEL_WARN,
        LOGLEVEL_INFO,
        LOGLEVEL_DEBUG,
        LOGLEVEL_LAST
    };
    /**
	* LogRecord
	*
	* @param  {string}              : 模块名
    * @param  {EnLogLevel}          : 日志等级
    * @param  {string}              : 消息内容
	*/
    USER_DEFINED(void, LogRecord, std::string, EnLogLevel, std::string);

	/**
	* LogIVICtrl
	*
	* @param  {int}              : bit0~3:日志导出请求(0x1=日志导出请求,0x2=日志导出完成,0x3=日志导出失败)，  bit4~7:日志开启/关闭请求(0x1=日志开启请求,0x2=日志关闭请求)
	*/
    USER_DEFINED(void, LogIVICtrl, int);

	/**
	* LogIVICtrl_Resp
	*
	* @param  {int}              : bit0~3:日志导出请求(0x1=日志导出中,0x2=日志准备完成,0x3=日志导出失败)，  bit4~7:日志开启/关闭请求(0x1=日志已开启,0x2=日志已关闭)
	*/
    USER_DEFINED(void, LogIVICtrl_Resp, int);

}//namespace logService
}//namespace ZH

#define LOGRECORDEHEX(buf, length, moudleName, param)            	\
		{															\
			const int cMaxLength = 100;								\
            int len = length > cMaxLength ? cMaxLength :length;/*太长就截断打印*/	\
            char tmpBuf[cMaxLength*3 + 50];                         		\
            snprintf(tmpBuf, 15, "[%s]", param);                       	\
			int paramLen = strlen(param) + 2; paramLen = paramLen > 15 ? 15 : paramLen;	\
            for (unsigned int i = 0; i < (unsigned int)(len); i++) 	\
                sprintf(&tmpBuf[paramLen + i * 3], "%02X ", buf[i]);         	\
            ZH::logService::LogRecord(moudleName, ZH::logService::EnLogLevel::LOGLEVEL_INFO, tmpBuf);	\
        }

#endif //!_LOGSERVICEAPI_H_