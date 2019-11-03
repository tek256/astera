MAKE=make
CC=clang
ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	MAKE_DIR := $(shell cd)
else
	MAKE_DIR := $(shell pwd)
endif

TARGET_COMPILER_FLAGS :=-w -std=c11 #-Werror -pedantic -Wall -Wextra

EXEC_NAME := astera

ifeq ($(OS),Windows_NT)
	RM_CMD := -del
	TARGET_LINKER_FLAGS := -I$(MAKE_DIR)\dep\ -I$(MAKE_DIR)\dep\glfw\include -I$(MAKE_DIR)\dep\openal-soft\include\ -I$(MAKE_DIR)\dep\zip\src\ -lopengl32 -lglfw -lgdi32 -lm -lopenal
	TARGET_EXEC_NAME := $(EXEC_NAME)
	TARGET_COMPILER_FLAGS += -D WIN32
	CC=gcc
else
	UNAME_S := $(shell uname -s)
	RM_CMD := -rm
	
	ifeq ($(UNAME_S),Linux)
		TARGET_LINKER_FLAGS := -I$(MAKE_DIR)/dep/ -lglfw -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lpthread -ldl -lXcursor -lm -lopenal -lzip
		TARGET_EXEC_NAME := $(EXEC_NAME)
		TARGET_COMPILER_FLAGS += -D LINUX
	else ifeq ($(UNAME_S),Darwin)
		TARGET_LINKER_FLAGS := -lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -ldl -lXcursor -lopenal -lm
		TARGET_EXEC_NAME := $(EXEC_NAME).app
		TARGET_COMPILER_FLAGS += -D OSX
	endif
endif

all : $(OBJS)
	$(CC) $(wildcard src/*.c) $(TARGET_COMPILER_FLAGS) $(TARGET_LINKER_FLAGS) -o $(TARGET_EXEC_NAME)	

PHONY: clean
clean :
	$(RM_CMD) $(TARGET_EXEC_NAME)
