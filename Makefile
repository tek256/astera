OBJS = $(wildcard src/*.c)

CC = clang
WIN_CC = gcc 

COMPILER_FLAGS = -w -std=c99 -O2
WIN_COMPILER_FLAGS = -w -std=c99
OSX_COMPILER_FLAGS = -w -std=c99

ifeq ($(OS),Windows_NT)
	SHELL = cmd.exe
	MAKE_DIR = $(shell cd)
else
	MAKE_DIR = $(shell pwd)
endif

INCLUDES = -I$(MAKE_DIR)/dep/openal-soft/include/ -I$(MAKE_DIR)/dep/glfw/include/ -I$(MAKE_DIR)/dep/ 

LINKER_FLAGS = -L$(MAKE_DIR)/dep/glfw/src/ -lglfw -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -ldl -lXcursor -lm -L$(MAKE_DIR)/dep/openal-soft/ -lopenal
WIN_LINKER_FLAGS = -lopengl32 -L$(MAKE_DIR)/dep/glfw/src/ -lglfw3 -lgdi32 -lm -L$(MAKE_DIR)/dep/openal-soft/ -lopenal32
OSX_LINKER_FLAGS = -lGL -lGLU -L$(MAKE_DIR)/dep/glfw/src/ -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -L$(MAKE_DIR/dep/openal-soft/ -lopenal -lm 

EXEC_NAME = engine
WIN_EXEC_NAME = $(EXEC_NAME).exe
OSX_EXEC_NAME = $(EXEC_NAME).app

ifeq ($(OS),Windows_NT)
	RM_CMD = -del
	TARGET_LINKER_FLAGS := $(WIN_LINKER_FLAGS)
	TARGET_COMPILER_FLAGS := $(WIN_COMPILER_FLAGS)
	TARGET_CC := $(WIN_CC)
	TARGET_EXEC_NAME := $(WIN_EXEC_NAME)
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
		TARGET_CC := $(CC)
		TARGET_COMPILER_FLAGS := $(COMPILER_FLAGS)
		TARGET_LINKER_FLAGS := $(LINKER_FLAGS)
		TARGET_EXEC_NAME := $(EXEC_NAME)
		TARGET_COMPILER_FLAGS += -D LINUX
	else ifeq ($(UNAME_S),Darwin)
		TARGET_CC := $(OSX_CC)
		TARGET_COMPILER_FLAGS := $(OSX_COMPILER_FLAGS)
		TARGET_LINKER_FLAGS := $(OSX_LINKER_FLAGS)
		TARGET_EXEC_NAME := $(OSX_EXEC_NAME)
		TARGET_COMPILER_FLAGS += -D OSX
	endif

	#Default to x86_64
	ifeq ($(UNAME_P),x86_64)
		TARGET_COMPILER_FLAGS += -march=x86-64
	else ifeq ($(UNAME_P),x86)
		TARGET_COMPILER_FLAGS += -march=i386
	else ifeq ($(UNAME_P),ARM)
		TARGET_COMPILER_FLAGS += -march=ARM
	else ifeq ($(UNAME_P),ARM64)
		TARGET_COMPILER_FLAGS += -march=ARM64
	else
		TARGET_COMPILER_FLAGS += -march=x86-64
	endif

endif

all : $(OBJS)
	$(TARGET_CC) $(OBJS) $(TARGET_COMPILER_FLAGS) $(TARGET_LINKER_FLAGS) $(INCLUDES) -o $(TARGET_EXEC_NAME)

.PHONY: clean
clean :
	$(RM_CMD) $(TARGET_EXEC_NAME)
