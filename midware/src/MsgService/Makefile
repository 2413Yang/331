PRJDIR ?= ../

ARTIFACT = MsgService

include ../../config.mk

INCLUDES += -I. -I./StrategySrc -I$(PRJDIR)../include 

ifeq ($(TARGET_FLAG), _QNX_TARGET_)
SUBDIR ?= $(call rwildcard, ./StrategySrc, c cpp)
SRCS = $(SUBDIR) MsgService.cpp
LIBS += -lIPCCore -lpps -lLogLib -lBaseLib -lCommunicDevice -lProtocolCodec -lUpgradeCore
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
SUBDIR ?= $(call rwildcard, ./StrategySrc, c cpp)
SRCS = MsgService.cpp $(SUBDIR) 
LIBS += -lIPCCore -lJsoncpp -lLogLib -lBaseLib -lCommunicDevice -lProtocolCodec  -Wl,--rpath,/usr/sbin/zhapp/midware/lib
endif

#HINSTALL +=

include ../../common.mk
