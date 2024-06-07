
#ifndef IPCCORE__H__
#define IPCCORE__H__

#include "IPC/InterfaceDefine.hpp"
#ifdef _QNX_TARGET_
#include "IPC/PPSConnector.h"
typedef CPPSConnector CIPCConnector;
#endif
#ifdef _LINUX_TARGET_
#include "IPC/NetConnector.h"
typedef CNetConnector CIPCConnector;
#endif

#endif /*IPCCORE__H__*/