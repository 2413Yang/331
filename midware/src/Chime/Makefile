PRJDIR ?= ../

ARTIFACT = ChimeService

INCLUDES += -I./ -I$(PRJDIR)../include -I$(SDKTARGETSYSROOT)/usr/include/glib-2.0/
INCLUDES += -I$(SDKTARGETSYSROOT)/usr/lib/glib-2.0/include/
INCLUDES += -I$(SDKTARGETSYSROOT)/usr/include/gstreamer-1.0/
INCLUDES += -I$(SDKTARGETSYSROOT)/usr/include/

LIBS += -lLogLib -lIPCCore -lBaseLib -lCommunicDevice -lJsoncpp -lpthread -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0

HINSTALL = ChimeApi.h

include ../../common.mk
