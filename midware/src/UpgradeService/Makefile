# TOPCUR:=$(dir $(abspath  $(lastword $(MAKEFILE_LIST))))
# PRJDIR := $(shell readlink -f $(TOPCUR)/../)
include ../../config.mk

ifeq ($(TARGET_FLAG), _QNX_TARGET_)
SUBDIRS = core service
else ifeq ($(TARGET_FLAG), _LINUX_TARGET_)
SUBDIRS = service
endif


SUBDIRHS = $(SUBDIRS)

SUBTARGET = build clean install  rebuild #hinstall

.PHONY: $(SUBDIRS)

all: 
	make install_head_file
	make build
	
build: CMD = build
rebuild: CMD = rebuild
install: CMD = install
hinstall:
	make install_head_file
clean: CMD = clean

$(SUBTARGET): $(SUBDIRS)

install_head_file:
	for dir in $(SUBDIRHS); 	\
	do  make -C $$dir hinstall; \
	done

$(SUBDIRS):
	@echo "<<< $@ : $(CMD) >>>"
	make -C $@ $(CMD)

