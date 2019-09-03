OBJS := $(wildcard src/*.c)

TARGET_CC := gcc
TARGET_COMPILER_FLAGS := -w -std=c99

ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	MAKE_DIR := $(shell cd)
else
	MAKE_DIR := $(shell pwd)
endif

INCLUDES := -I$(MAKE_DIR)/dep/ -I$(MAKE_DIR)/dep/openal-soft/include/ -I$(MAKE_DIR)/dep/glfw/include/

GLFW_OBJS := -L$(MAKE_DIR)/dep/glfw/src/
OPENAL_OBJS := -L$(MAKE_DIR)/dep/openal-soft/ 

SHARED_OBJS := $(GLFW_OBJS) $(OPENAL_OBJS)

UNIX_LINKER_FLAGS := -lglfw -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -ldl -lXcursor -lm -lopenal
WIN_LINKER_FLAGS := -lopengl32 -L$(MAKE_DIR)/dep/glfw/src/ -lglfw3 -lgdi32 -lm -L$(MAKE_DIR)/dep/openal-soft/ -lopenal32

EXEC_NAME := astera
RUNTIME_PATH := -Wl,-rpath=$(MAKE_DIR)/build/

ifeq ($(OS),Windows_NT)
	RM_CMD := -del
	TARGET_LINKER_FLAGS := $(WIN_LINKER_FLAGS)
	TARGET_EXEC_NAME := $(EXEC_NAME).exe
	TARGET_COMPILER_FLAGS += -D WIN32
else
	UNAME_S := $(shell uname -s)
	RM_CMD := -rm
	
	ifeq ($(UNAME_S),Linux)
		TARGET_LINKER_FLAGS := $(UNIX_LINKER_FLAGS)
		TARGET_EXEC_NAME := $(EXEC_NAME)
		TARGET_COMPILER_FLAGS += -D LINUX
	else ifeq ($(UNAME_S),Darwin)
		TARGET_LINKER_FLAGS := -lGL -lGLU -L$(MAKE_DIR)/dep/glfw/src/ -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -L$(MAKE_DIR/dep/openal-soft/ -lopenal -lm
		TARGET_EXEC_NAME := $(EXEC_NAME).app
		TARGET_COMPILER_FLAGS += -D OSX
	endif
endif

all : $(OBJS)
	$(TARGET_CC) $(SHARED_OBJS) $(TARGET_COMPILER_FLAGS) $(RUNTIME_PATH) -o $(TARGET_EXEC_NAME) $(OBJS) $(TARGET_LINKER_FLAGS) $(INCLUDES)

.PHONY: clean
clean :
	$(RM_CMD) $(TARGET_EXEC_NAME)
