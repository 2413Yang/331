PRJDIR ?= ../../

ARTIFACT = vpuplayer

INCLUDES += -I./  -I$(PRJDIR)../include/IPC  -I$(PRJDIR)../include/videoPlay

HINSTALL = videoPlayer_api.h
HINSTALLDIR = $(PRJDIR)../include/videoPlay/

LIBS += -lBaseLib -lJsoncpp -lLogLib -lIPCCore -lvpuplayer -lpthread -Wl,--rpath,/usr/sbin/zhapp/midware/lib

include ../../../common.mk