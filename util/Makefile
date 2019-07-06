OBJS = *.c

CC = clang
WIN_CC = mingw32-gcc

COMPILER_FLAGS = -w -std=c99

LINKER_FLAGS = -lminiz
EXEC_NAME = compress
WIN_EXEC_NAME = compress.exe

RM_CMD = -rm

ifeq ($(OS),Windows_NT)
    CC_FLAGS += -D WIN32
    RM_CMD = -del
	TARGET_LINKER_FLAGS := $(WIN_LINKER_FLAGS)
	TARGET_CC := $(WIN_CC)
	TARGET_EXEC_NAME := $(WIN_EXEC_NAME)
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        CC_FLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CC_FLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CC_FLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		TARGET_CC := $(CC)
		TARGET_LINKER_FLAGS := $(LINKER_FLAGS)
		TARGET_EXEC_NAME := $(EXEC_NAME)
        CC_FLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
		TARGET_CC := $(OSX_CC)
		TARGET_LINKER_FLAGS := $(OSX_LINKER_FLAGS)
		TARGET_EXEC_NAME := $(EXEC_NAME)
        CC_FLAGS += -D OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CC_FLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CC_FLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CC_FLAGS += -D ARM
    endif
endif

all : $(OBJS)
	$(TARGET_CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(TARGET_EXEC_NAME)

.PHONY: clean
clean :
		$(RM_CMD) $(TARGET_EXEC_NAME)
