ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	MAKE_DIR := $(shell cd)
	CC = clang
else
	MAKE_DIR := $(shell pwd)
	CC = clang 
endif

CC_FLAGS := -std=c99 -w

ifeq ($(OS),Windows_NT)
	LINKER_FLAGS := -I$(MAKE_DIR)../../src/ -Wl,-rpath=$(MAKE_DIR)/lib/ -lopengl32 -lglfw3 -lgdi32 -lm -lopenal32
	CC_FLAGS += -D WIN32
	OUT_EXTENSION := .exe
	RM_CMD := -del
else
	RM_CMD := -rm
	uname := $(shell uname)
	LINKER_FLAGS := -I$(MAKE_DIR)../../src/ -Wl,-rpath=$(MAKE_DIR)/lib/ -lglfw -lgl -lopenal -lm -lastera 
  OUT_EXTENSION := 

	ifeq($(uname),Linux)
		CC_FLAGS += -D Linux
	else ifeq ($(uname),FreeBSD)
		CC_FLAGS += -D FreeBSD
	else ifeq ($(uname),NetBSD)
		CC_FLAGS += -D NetBSD
	else ifeq ($(uname),OpenBSD)
		CC_FLAGS += -D OpenBSD
	endif
endif

.PHONY: all
all: $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard animations/*.c) $(LINKER_FLAGS) -o animations$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard audio/*.c) $(LINKER_FLAGS) -o audio$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard basic_game/*.c) $(LINKER_FLAGS) -o basic_game$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard basic_sprite/*.c) $(LINKER_FLAGS) -o basic_sprite$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard config/*.c) $(LINKER_FLAGS) -o config$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard input/*.c) $(LINKER_FLAGS) -o input$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard music/*.c) $(LINKER_FLAGS) -o music$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard render_stress/*.c) $(LINKER_FLAGS) -o render_stress$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard shaders/*.c) $(LINKER_FLAGS) -o shaders$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard sprites/*.c) $(LINKER_FLAGS) -o sprites$(OUT_EXTENSION)
	$(CC) $(CC_FLAGS) $(wildcard ui/*.c) $(LINKER_FLAGS) -o ui$(OUT_EXTENSION)

.PHONY: animations
animations : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard animations/*.c) $(LINKER_FLAGS) -o animations$(OUT_EXTENSION)

.PHONY: audio
audio : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard audio/*.c) $(LINKER_FLAGS) -o audio$(OUT_EXTENSION)

.PHONY: basic_game
basic_game : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard basic_game/*.c) $(LINKER_FLAGS) -o basic_game$(OUT_EXTENSION)

.PHONY: basic_sprite
basic_sprite : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard basic_sprite/*.c) $(LINKER_FLAGS) -o basic_sprite$(OUT_EXTENSION)

.PHONY: config
config : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard config/*.c) $(LINKER_FLAGS) -o config$(OUT_EXTENSION)

.PHONY: input
input : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard input/*.c) $(LINKER_FLAGS) -o input$(OUT_EXTENSION)

.PHONY: music
music : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard music/*.c) $(LINKER_FLAGS) -o music$(OUT_EXTENSION)

.PHONY: render_stress
render_stress : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard render_stress/*.c) $(LINKER_FLAGS) -o render_stress$(OUT_EXTENSION)

.PHONY: shaders
shaders : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard shaders/*.c) $(LINKER_FLAGS) -o shaders$(OUT_EXTENSION)

.PHONY: sprites
sprites : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard sprites/*.c) $(LINKER_FLAGS) -o sprites$(OUT_EXTENSION)

.PHONY: ui
ui : $(OBJS)
	$(CC) $(CC_FLAGS) $(wildcard ui/*.c) $(LINKER_FLAGS) -o ui$(OUT_EXTENSION)

.PHONY: clean
	$(RM_CMD) ui$(OUT_EXTENSION)
	$(RM_CMD) sprites$(OUT_EXTENSION)
	$(RM_CMD) shaders$(OUT_EXTENSION)
	$(RM_CMD) render_stress$(OUT_EXTENSION)
	$(RM_CMD) music$(OUT_EXTENSION)
	$(RM_CMD) input$(OUT_EXTENSION)
	$(RM_CMD) config$(OUT_EXTENSION)
	$(RM_CMD) basic_sprite$(OUT_EXTENSION)
	$(RM_CMD) basic_game$(OUT_EXTENSION)
	$(RM_CMD) audio$(OUT_EXTENSION)
	$(RM_CMD) animations$(OUT_EXTENSION)
