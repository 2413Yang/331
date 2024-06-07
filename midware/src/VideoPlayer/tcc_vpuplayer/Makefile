PRJDIR ?= ../../

include ../../../config.mk


ARTIFACT = libvpuplayer.so
INCLUDES += -I./ -I./common/ -I./include
LIBS += 

HINSTALL = ./tcc_vdec_api.h
HINSTALLDIR = $(PRJDIR)../include/videoPlay/

TOP := ./
COMMON_PATH 		:= $(TOP)common/
LOCAL_PATH 			:= $(TOP)

SRCS += $(COMMON_PATH)TCCMemory.c
SRCS += $(LOCAL_PATH)pmap.c
SRCS += $(LOCAL_PATH)vdec_k.c
SRCS += $(LOCAL_PATH)vpu_dec.c
SRCS += $(LOCAL_PATH)tcc_vdec_api.c

include ../../../common.mk



