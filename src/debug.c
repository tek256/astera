#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#define STR_BUFF_SIZE 128

static int logging = 0;
static char* log_fp;

static int refresh_rate = 5;
static int changes_made = 0;

void dbg_set_log_fp(const char* fp){
   log_fp = fp; 
   FILE* f = fopen(fp, "w");
   fwrite("\0", sizeof(char), 1, f); 
   fclose(f);
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

void dbg_enable_log(int log){
    if(log != logging){
        logging = log;

        if(log_fp){
            if(logging){
                FILE* chk = fopen(log_fp, "a");
                if(!chk){
                    logging = 0;
                    dbg_log("Unable to open file: %s, file doesn't exist.\n", log_fp);
                }
				fclose(chk);
            }
        }
    }
}

static char strbuff[STR_BUFF_SIZE];
int dbg_log(const char* format, ...){
    va_list args;
    va_start(args, format);

    if(logging){
        vsprintf(strbuff, format, args);

        FILE* f = fopen(log_fp, "a");        
        fwrite(strbuff, sizeof(char), strlen(strbuff), f);    
        fclose(f);

        memset(strbuff, 0, sizeof(char) * STR_BUFF_SIZE);
    }

    vprintf(format, args);
    va_end(args);
    
    return 1;
}

int dbg_post_to_err(){
    char str_buff[STR_BUFF_SIZE];
    //no logging set up
    if(!log_fp){
        return 0;
    }
    //@TODO Time String Implementation
    char time_buff[64];
    time_t raw;
    struct tm* ti;
    time(&raw);
    ti = localtime(&raw);

    strftime(time_buff, 64, "%d%Om%Y_%H%M%S", ti);

    memset(str_buff, 0, sizeof(char) * STR_BUFF_SIZE); 
    strcat(str_buff, "ERR_");
    strcat(str_buff, time_buff);
    strcat(str_buff, ".txt");
    strcat(str_buff, "\0");
    
    FILE* o = fopen(str_buff, "w");
    FILE* i = fopen(log_fp, "r");

    if(!o){
        dbg_log("Unable to open, %s for error output.\n", log_fp);
        fclose(o);
        fclose(i);
        memset(str_buff, 0, sizeof(char) * STR_BUFF_SIZE);
        return 0;
    }

    if(!i){
        dbg_log("Unable to open, %s for error output.\n", strbuff);
        fclose(o);
        fclose(i);
        memset(str_buff, 0, sizeof(char) * STR_BUFF_SIZE);
        return 0;
    }
    
    memset(strbuff, 0, sizeof(char) * STR_BUFF_SIZE);
    fseek(i, 0, SEEK_END); 
    long size = ftell(i);   
    long rem = size;

    rewind(i);
    
    while(rem > 0){
        rem -= fread(str_buff, sizeof(char), STR_BUFF_SIZE, i);
        fwrite(str_buff, sizeof(char), STR_BUFF_SIZE, o);
    }

    fclose(i);
    fclose(o);

    memset(str_buff, 0, sizeof(char) * STR_BUFF_SIZE);
   
    dbg_cleanup();
    return 1;
}
