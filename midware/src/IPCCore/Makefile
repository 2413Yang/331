PRJDIR ?= ../

ARTIFACT = libIPCCore.so

include ../../config.mk

INCLUDES += -I$(PRJDIR)../include -I.


ifeq ($(TARGET_FLAG), _QNX_TARGET_)
LIBS += -lpps -lLogLib -lCommunicDevice
SRCS =  PPSConnector.cpp  \
		PPSNodeDevice.cpp \
		SerializePPS.cpp 
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
LIBS += -lLogLib -lCommunicDevice -lJsoncpp
SRCS =	NetConnector.cpp \
		BaseTypeJson.cpp \
		CustomTypeJson.cpp 
endif

HINSTALLDIR = $(PRJDIR)../include/IPC

HINSTALL = InterfaceDefine.hpp ArgumentsTable.hpp IPCCallback.hpp PPSConnector.h  TemplateTypeJson.hpp\
		   SerializePPS.h PPSNodeDevice.h CustomTypeJson.h BaseTypeJson.h NetConnector.h IPCCore.h

include ../../common.mk
