OBJS = $(wildcard src/*.c)

UNIX_CC = clang
WIN_CC = clang

#DEBUG FLAGS
#COMPILER_FLAGS = -D_FORTIFY_SOURCE=2 -fstack-clash-protection -g -O2 -Werror=format-security -std=c99 -l 2.5

UNIX_COMPILER_FLAGS = -w -std=c99
WIN_COMPILER_FLAGS = -w -std=c99

ifeq ($(OS),Windows_NT)
	SHELL = cmd.exe
	MAKE_DIR = $(shell cd)
else
	MAKE_DIR = $(shell pwd)
endif

INCLUDES = -I$(MAKE_DIR)/dep/ -I$(MAKE_DIR)/dep/openal-soft/include/ -I$(MAKE_DIR)/dep/glfw/include/

UNIX_LINKER_FLAGS = -L$(MAKE_DIR)/dep/glfw/src/ -lglfw -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -ldl -lXcursor -lm -L$(MAKE_DIR)/dep/openal-soft/ -lopenal
WIN_LINKER_FLAGS = -lopengl32 -L$(MAKE_DIR)/dep/glfw/src/ -lglfw3 -lgdi32 -lm -L$(MAKE_DIR)/dep/openal-soft/ -lopenal32
OSX_LINKER_FLAGS = -lGL -lGLU -L$(MAKE_DIR)/dep/glfw/src/ -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -L$(MAKE_DIR/dep/openal-soft/ -lopenal -lm 

UNIX_EXEC_NAME = astera 

ifeq ($(OS),Windows_NT)
	RM_CMD = -del
	TARGET_LINKER_FLAGS := $(WIN_LINKER_FLAGS)
	TARGET_COMPILER_FLAGS := $(WIN_COMPILER_FLAGS)
	TARGET_CC := $(WIN_CC)
	TARGET_EXEC_NAME := $(UNIX_EXEC_NAME).exe
	TARGET_COMPILER_FLAGS += -D WIN32

	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		TARGET_COMPILER_FLAGS += -march=x86-64
	else ifeq ($(PROCESSOR_ARCHITECTURE),x86)
		TARGET_COMPILER_FLAGS += -march=i386
	endif
else
	UNAME_S := $(shell uname -s)
	UNAME_P := $(shell uname -p)
	RM_CMD = -rm
	
	ifeq ($(UNAME_S),Linux)
		TARGET_CC := $(UNIX_CC)
		TARGET_COMPILER_FLAGS := $(UNIX_COMPILER_FLAGS)
		TARGET_LINKER_FLAGS := $(UNIX_LINKER_FLAGS)
		TARGET_EXEC_NAME := $(UNIX_EXEC_NAME)
		TARGET_COMPILER_FLAGS += -D LINUX
	else ifeq ($(UNAME_S),Darwin)
		TARGET_CC := $(UNIX_CC)
		TARGET_COMPILER_FLAGS := $(UNIX_COMPILER_FLAGS)
		TARGET_LINKER_FLAGS := $(OSX_LINKER_FLAGS)
		TARGET_EXEC_NAME := $(UNIX_EXEC_NAME).app
		TARGET_COMPILER_FLAGS += -D OSX
	endif
endif

all : $(OBJS)
	$(TARGET_CC) $(OBJS) $(TARGET_COMPILER_FLAGS) $(TARGET_LINKER_FLAGS) $(INCLUDES) -o $(TARGET_EXEC_NAME)

.PHONY: clean
clean :
	$(RM_CMD) $(TARGET_EXEC_NAME)
