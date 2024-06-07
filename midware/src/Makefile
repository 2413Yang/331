# TOPCUR:=$(dir $(abspath  $(lastword $(MAKEFILE_LIST))))
# PRJDIR := $(shell readlink -f $(TOPCUR)/../)
include ../config.mk

HEADDIRS = LogLib BaseLib CommunicDevice ProtocolCodec IPCCore \
	LogService UpgradeService WatchService  Chime MsgService

ifeq ($(TARGET_FLAG), _QNX_TARGET_)
SUBDIRS= ${HEADDIRS}
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
SUBDIRS= LogLib BaseLib CommunicDevice ProtocolCodec IPCCore LogService WatchService UpgradeService VideoPlayer Chime MsgService HUMsgService
endif



SUBDIRHS = $(SUBDIRS)

SUBTARGET = build clean install  rebuild #hinstall

.PHONY: $(SUBDIRS)

all: 
	make install_out_head_file
	make install_head_file
	make build
	
install_out_head_file:
	mkdir -p ../include/hmi/
	cp -R $(KANZI_IC_DIR)/Application/src/WarnIdDefine.h ../include/hmi/
	cp -R $(KANZI_IC_DIR)/Application/src/MsgInterface.h ../include/hmi/
	cp -R $(KANZI_IC_DIR)/Application/src/PageIdDefine.h ../include/hmi/

ins_tag:
	cp -fr ../aarch64le/* $(IMG_OUTPUT_DIR)/midware/

build: CMD = build
rebuild: CMD = rebuild
install: CMD = install
hinstall: CMD = hinstall
clean: CMD = clean

$(SUBTARGET): $(SUBDIRS)

install_head_file:
	for dir in $(HEADDIRS); 		\
	do  make -C $$dir hinstall; \
	done

$(SUBDIRS):
	@echo "<<< $@ : $(CMD) >>>"
	make -C $@ $(CMD)

