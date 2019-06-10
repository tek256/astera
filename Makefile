OBJS = src/main.c src/audio.c src/input.c src/render.c src/sys.c src/geom.c src/debug.c src/game.c src/platform.c

CC = gcc
WIN_CC = mingw32-gcc

COMPILER_FLAGS = -w -std=c11
WIN_COMPILER_FLAGS = -w -std=c11
OSX_COMPILER_FLAGS = -w -std=c11

INCLUDES = -Idep/

LINKER_FLAGS = -lglfw3 -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -ldl -lXcursor -lopenal -lm -lGLU
WIN_LINKER_FLAGS = -lopengl32 -lglfw3 -lgdi32 -lm -lopenal32
OSX_LINKER_FLAGS = -lGL -lGLU -lglfw3  -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -lopenal -lm 
EXEC_NAME = engine
WIN_EXEC_NAME = $(EXEC_NAME).exe
OSX_EXEC_NAME = $(EXEC_NAME)

RM_CMD = -rm

ifeq ($(OS),Windows_NT)
    CC_FLAGS += -D WIN32
    RM_CMD = -del
	TARGET_LINKER_FLAGS := $(WIN_LINKER_FLAGS)
	TARGET_COMPILER_FLAGS := $(WIN_COMPILER_FLAGS)
	TARGET_CC := $(WIN_CC)
	TARGET_EXEC_NAME := $(WIN_EXEC_NAME)
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
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
		TARGET_COMPILER_FLAGS := $(COMPILER_FLAGS)
		TARGET_LINKER_FLAGS := $(LINKER_FLAGS)
		TARGET_EXEC_NAME := $(EXEC_NAME)
        CC_FLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
		TARGET_CC := $(OSX_CC)
		TARGET_COMPILER_FLAGS := $(OSX_COMPILER_FLAGS)
		TARGET_LINKER_FLAGS := $(OSX_LINKER_FLAGS)
		TARGET_EXEC_NAME := $(OSX_EXEC_NAME)
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
	$(TARGET_CC) $(OBJS) $(TARGET_COMPILER_FLAGS) $(TARGET_LINKER_FLAGS) $(INCLUDES) -o $(TARGET_EXEC_NAME)

.PHONY: clean
clean :
		$(RM_CMD) $(TARGET_EXEC_NAME)
