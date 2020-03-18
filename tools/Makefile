CC=clang

ifeq ($(OS),Windows_NT)
  sampler_exec := sampler.exe
else
  sampler_exec := sampler
endif

all : $(OBJS)
	$(CC) src/audio_sampler.c -I../dep/misc/ -lm -o $(sampler_exec)
