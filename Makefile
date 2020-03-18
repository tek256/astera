ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	MAKE_DIR := $(shell cd)
	CC = clang
else
	MAKE_DIR := $(shell pwd)
	CC = clang 
endif

TARGET_COMPILER_FLAGS := -std=c99 -w #-O2 #-ferror-limit=500#-Wall -pedantic -Wextra -ferror-limit=500
EXEC_NAME := astera
OBJS := $(wildcard src/*.c)	
UNAME_S := $(shell uname)

# Windows Native
ifeq ($(OS),Windows_NT)
	RM_CMD := -del
	TARGET_LINKER_FLAGS := -I$(MAKE_DIR)\dep\ -I$(MAKE_DIR)\dep\glfw\include -I$(MAKE_DIR)\dep\openal-soft\include\ -L$(MAKE_DIR)\dep\glfw\src -L$(MAKE_DIR)\dep\openal-soft\ -lopengl32 -lglfw3 -lgdi32 -lm -lopenal32
	TARGET_EXEC_NAME := $(EXEC_NAME)
	#TARGET_COMPILER_FLAGS += -D WIN32
# Unix / BSD
else
	RM_CMD := -rm
	TARGET_EXEC_NAME := $(EXEC_NAME)
	
	LINKAGE := -I$(MAKE_DIR)/dep/ -I$(MAKE_DIR)/dep/glfw/include -I$(MAKE_DIR)/dep/openal-soft/include/ -L$(MAKE_DIR)/lib -Wl,-rpath,\$$ORIGIN/lib/ #-l:libglfw.so -l:libopenal.so# -Wl,-rpath,\$$ORIGIN/lib/
	LIBRARIES := -lglfw -lGL -lopenal -lm

	TARGET_LINKER_FLAGS := $(LINKAGE) $(LIBRARIES) 

  ifeq ($(UNAME_S),Linux)
		#TARGET_COMPILER_FLAGS += -D LINUX
  # Mac OSX
	else ifeq ($(UNAME_S),Darwin)
		TARGET_LINKER_FLAGS := -I$(MAKE_DIR)/dep/ -I/usr/local/include/ -L/usr/local/lib -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework CoreVideo -lopenal -lm -lzip -stdlib=libc++
		TARGET_COMPILER_FLAGS += -D OSX
	# FreeBSD
	else ifeq ($(UNAME_S),FreeBSD)
		TARGET_COMPILER_FLAGS += -D FreeBSD
	# NetBSD
	else ifeq ($(UNAME_S),NetBSD)
		TARGET_COMPILER_FLAGS += NetBSD
	# OpenBSD
	else ifeq ($(UNAME_S),OpenBSD)
		TARGET_COMPILER_FLAGS += OpenBSD
	endif
endif

all : $(OBJS)
	$(CC) $(OBJS) $(TARGET_COMPILER_FLAGS) $(TARGET_LINKER_FLAGS) -o $(TARGET_EXEC_NAME)	

PHONY: clean
clean :
	$(RM_CMD) $(TARGET_EXEC_NAME)
