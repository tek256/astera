OBJS = $(wildcard src/*.c)

UNIX_CC = clang
WIN_CC = clang

UNIX_COMPILER_FLAGS = -w -std=c99
WIN_COMPILER_FLAGS = -w -std=c99

ifeq ($(OS),Windows_NT)
	SHELL = cmd.exe
	MAKE_DIR = $(warning call to shell)$(shell cd)
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
else
	UNAME_S := $(shell uname -s)
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
