all : $(OBJS)
	clang src/zip.c -lzip -o zipper
	clang src/unzip.c -lzip -o unzipper
	clang src/audio_sampler.c -I../dep/stb -lm  -o audio_sampler
