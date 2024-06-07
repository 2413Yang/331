PRJDIR ?= ../

ARTIFACT = libBaseLib.so

INCLUDES += -I../../include

include ../../config.mk
ifeq ($(TARGET_FLAG), _QNX_TARGET_)
LIBS += -lLogLib
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
LIBS += -lLogLib -lrt -pthread  
endif


HINSTALL = Application.h TemplateThread.h Mutex.h Timer.h Semaphore.hpp TemplateTimer.h Hook.h TemplateCache.hpp FileL.hpp DoShellCmd.hpp simpleTimer.h STimer.h MemoryDataManager.h

include ../../common.mk
