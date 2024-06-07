PRJDIR ?= ../

ARTIFACT = HUMsgService

include ../../config.mk

INCLUDES += -I. -I./protocol -I./communic -I./server -I$(PRJDIR)../include 

ifeq ($(TARGET_FLAG), _QNX_TARGET_)
SUBDIR_PROTOCOL ?= $(call rwildcard, ./protocol, c cpp)
SUBDIR_COMMUNIC ?= $(call rwildcard, ./communic, c cpp)
SUBDIR_SERVER ?= $(call rwildcard, ./server, c cpp)
SRCS = $(SUBDIR_PROTOCOL) $(SUBDIR_COMMUNIC) $(SUBDIR_SERVER)
LIBS += -lIPCCore -lpps -lLogLib -lBaseLib -lCommunicDevice -lProtocolCodec -lUpgradeCore
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
SUBDIR_PROTOCOL ?= $(call rwildcard, ./protocol, c cpp)
SUBDIR_COMMUNIC ?= $(call rwildcard, ./communic, c cpp)
SUBDIR_SERVER ?= $(call rwildcard, ./server, c cpp)
SRCS = $(SUBDIR_PROTOCOL) $(SUBDIR_COMMUNIC) $(SUBDIR_SERVER)
LIBS += -lIPCCore -lJsoncpp -lLogLib -lBaseLib -lCommunicDevice -lProtocolCodec  -Wl,--rpath,/usr/sbin/zhapp/midware/lib
endif

HINSTALL += ./server/HUMsgDefine.h

include ../../common.mk
