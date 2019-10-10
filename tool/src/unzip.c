#include <stdio.h>
#include <stdlib.h>
#include <zip/zip.h>

int main(int argc, char** argv){
	if(argc == 1){
		printf("You must pass a zip archive as an argument\n");
		return EXIT_FAILURE;
	}

	struct zip_t* zip = zip_open(argv[1], 0, 'r');
	int i, n = zip_total_entries(zip);
	for(i=0;i<n;++i){
		zip_entry_openbyindex(zip, i);

		const char* name = zip_entry_name(zip);
		int isdir = zip_entry_isdir(zip);
		unsigned long long size = zip_entry_size(zip);

		printf("Unzipping [%lli] : %s\n", size, name);

		unsigned char* data = malloc(size);
		zip_entry_noallocread(zip, (void*)data, size);
			
		FILE* f = fopen(name, "w+");
		fwrite(data, size, sizeof(unsigned char), f);
		fclose(f);

		free(data);
			
		zip_entry_close(zip);
	}

	zip_close(zip);


	return EXIT_SUCCESS;
}
