OBJS := $(wildcard src/*.c)

TARGET_CC := clang
TARGET_COMPILER_FLAGS := -w -std=c11 #-g -ferror-limit=500 -Wall -Wextra -pedantic -std=c11

ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	MAKE_DIR := $(shell cd)
else
	MAKE_DIR := $(shell pwd)
endif

INCLUDES := -I$(MAKE_DIR)/dep/ #-I$(MAKE_DIR)/dep/openal-soft/include/ -I$(MAKE_DIR)/dep/glfw/include/ #-I$(MAKE_DIR)/dep/microui/
DYN_FLAGS := -Wl,-rpath=$(MAKE_DIR)/ -L$(MAKE_DIR)/dep/glfw/src/ -L$(MAKE_DIR)/dep/openal-soft/

EXEC_NAME := astera

ifeq ($(OS),Windows_NT)
	RM_CMD := -del
	TARGET_LINKER_FLAGS := -lopengl32 -lglfw3 -lgdi32 -lm -lopenal32 -lzip
	TARGET_EXEC_NAME := $(EXEC_NAME).exe
	TARGET_COMPILER_FLAGS += -D WIN32
else
	UNAME_S := $(shell uname -s)
	RM_CMD := -rm
	
	ifeq ($(UNAME_S),Linux)
		TARGET_LINKER_FLAGS := -lglfw3 -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -ldl -lXcursor -lm -lopenal -lzip
		TARGET_EXEC_NAME := $(EXEC_NAME)
		TARGET_COMPILER_FLAGS += -D LINUX
	else ifeq ($(UNAME_S),Darwin)
		TARGET_LINKER_FLAGS := -lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -lopenal -lm -lzip
		TARGET_EXEC_NAME := $(EXEC_NAME).app
		TARGET_COMPILER_FLAGS += -D OSX
	endif
endif

all : $(OBJS)
	$(TARGET_CC) $(OBJS) $(TARGET_COMPILER_FLAGS) $(TARGET_LINKER_FLAGS) $(INCLUDES) -o $(TARGET_EXEC_NAME)	

PHONY: dynamic
dynamic : $(OBJS)
	$(TARGET_CC) $(DYN_FLAGS) $(TARGET_COMPILER_FLAGS) -o $(TARGET_EXEC_NAME) $(OBJS) $(TARGET_LINKER_FLAGS) $(INCLUDES)

PHONY: clean
clean :
	$(RM_CMD) $(TARGET_EXEC_NAME)
