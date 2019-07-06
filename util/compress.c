#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/unistd.h>

#include <miniz/miniz.h>

int traverse(const char* dir);

int traverse(const char* dir){
	struct dirent* d;
	DIR* dr = opendir(dir);
	if(!dr){
		printf("Unable to open directory: %s\n", dir);
		return -1;
	}

	while(d = readdir(dr)){
		if(strlen(d->d_name) > 2){
			struct stat s;
			if(stat(d->d_name, &s) != -1){
				if(S_ISDIR(s.st_mode)){
					traverse(d->d_name);
				}else{
					printf("uhhhhhhhhhhhhhh %s\n", d->d_name);
				}	
			}else{
				printf("Unable to open directory: %s\n", d->d_name);
				FILE* f = fopen(d->d_name, "r");
				if(f){
					printf("We can open the file tho.\n");
					fclose(f);
				}
			}
		}
	}
	closedir(dr);
	return 1;
}

int main(int argc, char** argv){
	if(argc > 1){
		for(int i=1;i<argc;++i){
			struct stat s;
			if(stat(argv[i], &s) == -1){
				printf("Unable to stat check: %s\n", argv[i]);
			}else if(S_ISDIR(s.st_mode)){
				traverse(argv[i]);
			}
		}	
	}

	return EXIT_SUCCESS;
}
