#include "debug.h"
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define STR_BUFF_SIZE 128

static int logging = -1;
static int timestamp = 1;
static char* log_fp;

static int refresh_rate = 5;
static int changes_made = 0;

static char time_buff[64];

void dbg_set_log_fp(const char* fp){
   log_fp = fp; 
}

void dbg_set_timestamp(int enabled){
	timestamp = enabled;
}

int dbg_get_logging(){
    return logging && log_fp;
}

int dbg_cleanup(){
    if(log_fp){
        return remove(log_fp);
    }
    return 1;
}

void dbg_enable_log(int log, const char* fp){
	if(log != logging){
		logging = log;
	}

	if(fp && log){
		if(access(fp, F_OK) != -1){
			remove(fp);
		}

		FILE* chk = fopen(fp, "a");
		if(!chk){
			logging = 0;
			_e("Unable to open file: %s\n", fp);
		}
		fclose(chk);

		log_fp = fp;
	}
}

static char strbuff[STR_BUFF_SIZE];
void _l(const char* format, ...){
    va_list args;
    va_start(args, format);

	vsprintf(strbuff, format, args);
	if(!c_is_silent())
		fprintf(stdout, "%s", strbuff);
	int len = strlen(strbuff);

	if(logging){
		if(timestamp){
			time_t raw;
			struct tm* ti;
			time(&raw);
			ti = localtime(&raw);

			strftime(time_buff, 64, "%H:%M:%S ", ti);

			int ts_len = strlen(time_buff);
			
			FILE* f = fopen(log_fp, "a");
			fwrite(time_buff, sizeof(char), ts_len, f);
			fwrite(strbuff, sizeof(char), len, f);
			fclose(f);
			memset(time_buff, 0, sizeof(char) * 64);
		}else{
			FILE* f = fopen(log_fp, "a");        
			fwrite(strbuff, sizeof(char), len, f);    
			fclose(f);
		}
	}

	memset(strbuff, 0, sizeof(char) * STR_BUFF_SIZE);
	va_end(args);
}

void _fatal(const char* format, ...){
	va_list args;
	va_start(args, format);

	vsprintf(strbuff, format, args);
	fprintf(stderr, "FATAL: %s", strbuff);
	int len = strlen(strbuff);

	if(logging){
			if(timestamp){
			time_t raw;
			struct tm* ti;
			time(&raw);
			ti = localtime(&raw);

			strftime(time_buff, 64, "%H:%M:%S ", ti);

			int ts_len = strlen(time_buff);

			FILE* f = fopen(log_fp, "a");
			fwrite(time_buff, sizeof(char), ts_len, f);
			fwrite(strbuff, sizeof(char), len, f);
			fclose(f);
			memset(time_buff, 0, sizeof(char) * 64);
		}else{
			FILE* f = fopen(log_fp, "a");        
			fwrite(strbuff, sizeof(char), len, f);    
			fclose(f);
		}
	}

	memset(strbuff, 0, sizeof(char) * STR_BUFF_SIZE);
	va_end(args);
   
	d_fatal = 1;
}

void _e(const char* format, ...){
	va_list args;
	va_start(args, format);

	vsprintf(strbuff, format, args);
	fprintf(stderr, "%s", strbuff);
	int len = strlen(strbuff);

	if(logging){
		if(timestamp){
			time_t raw;
			struct tm* ti;
			time(&raw);
			ti = localtime(&raw);

			strftime(time_buff, 64, "%H:%M:%S ", ti);

			int ts_len = strlen(time_buff);

			FILE* f = fopen(log_fp, "a");
			fwrite(time_buff, sizeof(char), ts_len, f);
			fwrite(strbuff, sizeof(char), len, f);
			fclose(f);
			memset(time_buff, 0, sizeof(char) * 64);
		}else{
			FILE* f = fopen(log_fp, "a");        
			fwrite(strbuff, sizeof(char), len, f);    
			fclose(f);
		}
	}

	memset(strbuff, 0, sizeof(char) * STR_BUFF_SIZE);
	va_end(args);
}

int dbg_post_to_err(){
    char err_buff[STR_BUFF_SIZE];
    //no logging set up
    if(!log_fp){
        return 0;
    }
    
    time_t raw;
    struct tm* ti;
    time(&raw);
    ti = localtime(&raw);

    strftime(time_buff, 64, "%d%Om%Y_%H%M", ti);

    memset(err_buff, 0, sizeof(char) * STR_BUFF_SIZE); 
    strcat(err_buff, "ERR_");
    strcat(err_buff, time_buff);
    strcat(err_buff, ".txt");
    strcat(err_buff, "\0");
    
    FILE* o = fopen(err_buff, "w");
    FILE* i = fopen(log_fp, "r");

    if(!o){
        _e("Unable to open, %s for error output.\n", log_fp);
        fclose(o);
        fclose(i);
        memset(err_buff, 0, sizeof(char) * STR_BUFF_SIZE);
        return 0;
    }

    if(!i){
        _e("Unable to open, %s for error output.\n", err_buff);
        fclose(o);
        fclose(i);
        memset(err_buff, 0, sizeof(char) * STR_BUFF_SIZE);
        return 0;
    }
    
    memset(err_buff, 0, sizeof(char) * STR_BUFF_SIZE);

    fseek(i, 0, SEEK_END); 
    long size = ftell(i);   
    long rem = size-1;
    rewind(i);
    
    while(rem > 0){
        rem -= fread(err_buff, sizeof(char), STR_BUFF_SIZE, i);
        fwrite(err_buff, sizeof(char), STR_BUFF_SIZE, o);
    }

	memset(time_buff, 0, sizeof(char) * 64);

    fclose(i);
    fclose(o);

    memset(err_buff, 0, sizeof(char) * STR_BUFF_SIZE);
   
    dbg_cleanup();
    return 1;
}
