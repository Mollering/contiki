CONTIKI = ../..

ifndef TARGET
TARGET = sky
endif

CONTIKI_PROJECT = rime-mobiwise

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += collect-common.c

CONTIKI_WITH_RIME = 1

MODULES += 

TARGET_LIBFILES += -lm

APPS += powertrace collect-view

all: sensor-node sink-node

include $(CONTIKI)/Makefile.include
