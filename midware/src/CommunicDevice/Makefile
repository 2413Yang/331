PRJDIR ?= ../

ARTIFACT = libCommunicDevice.so

include ../../config.mk

INCLUDES += -I./

ifeq ($(TARGET_FLAG), _QNX_TARGET_)
LIBS += -lLogLib -lsocket
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
LIBS += -lLogLib
endif

HINSTALL = CommunicDevice.h NetDevice.h SpiDevice.h UartDevice.h

include ../../common.mk
