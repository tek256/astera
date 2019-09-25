#include "asset.h"

asset_t load_asset(const char* archive, const char* filename){
	struct zip_t* zip = zip_open(archive, 0, 'r');
	zip_entry_open(zip, filename);

	unsigned int file_size = zip_entry_size(zip);
	unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * file_size);

	zip_entry_noallocread(zip, (void*)data, file_size);
	zip_entry_close(zip);
	zip_close(zip);

	return (asset_t){0, data, file_size, filename};
}
