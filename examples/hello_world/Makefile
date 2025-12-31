CC = gcc
PYTHON ?= python3

WCE_ROOT = ../..
UI_FILE = ui/pure_c.wce
GEN_DIR = generated

# 让 webcee.h 自动包含生成的声明（WCE_EVT_* / WCE_MODEL_* 等）
CFLAGS = -I$(WCE_ROOT)/include -I./$(GEN_DIR) -DWEBCEE_HAS_GENERATED=1
SRCS = src/main.c $(GEN_DIR)/webcee_generated.c $(WCE_ROOT)/src/webcee.c

ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
else
    LDFLAGS += -lpthread
endif

all:
	$(PYTHON) $(WCE_ROOT)/tools/wce_parser.py build $(UI_FILE) -o $(GEN_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o firmware

clean:
	rm -f firmware *.o
	rm -rf $(GEN_DIR)
