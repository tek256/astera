CC=clang

zipper_name := zipper
unzip_name := unzipper
sampler_name := sampler

ifeq ($(OS),Windows_NT)
	zipper_name += .exe
	unzip_name += .exe
	sampler_name += .exe
endif

all : $(OBJS)
	$(CC) src/zip.c -lzip -o $(zipper_name)
	$(CC) src/unzip.c -lzip -o $(unzip_name)
	$(CC) src/audio_sampler.c -I../dep/misc -lm  -o $(sampler_name)

clean:
ifeq ($(PLATFORM),DESKTOP)
	ifeq ($(OS),WINDOWS)
		del *.o *.exe /s
	endif
	ifeq ($(OS),LINUX)
		find -type f -executable | xargs file -i | grep -E 'x-archive|x-sharedlib|x-executable|x-object|x-pie-executable' | rev | cut -d ':' -f 2- | rev | xargs rm -fv
	endif
	ifeq ($(OS),OSX)
		find . -type f -perm +ugo+x -delete
		rm -f *.o
	endif
endif
@echo Tool files cleaned
