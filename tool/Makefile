CC=clang

ifeq ($(OS),Windows_NT)
	zipper_exec := zipper.exe
  unzipper_exec := unzipper.exe
  sampler_exec := sampler.exe
else
	zipper_exec := zipper
  unzipper_exec := unzipper
  sampler_exec := sampler

endif

all : $(OBJS)
	$(CC) src/zip.c -lzip -o $(zipper_exec)
	$(CC) src/unzip.c -lzip -o $(unzipper_exec)
	$(CC) src/audio_sampler.c -I../dep/misc -lm  -o $(sampler_exec)
