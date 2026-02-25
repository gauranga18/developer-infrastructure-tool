#include "log.h"
#include "config.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
static int get_level_value(const char *level){
    if(strcmp(level,"ERROR")==0)return LOG_LEVEL_ERROR;
    if(strcmp(level,"WARN")==0)return LOG_LEVEL_WARN;
    if(strcmp(level,"INFO")==0)return LOG_LEVEL_INFO;
    if(strcmp(level,"DEBUG")==0)return LOG_LEVEL_DEBUG;
    return LOG_LEVEL_INFO;
}
static void write_log(const char *level,const char *message){
    int msg_level = get_level_value(level);
    if(msg_level > g_config.log_level){
        return;
    }
time_t now = time(NULL);
struct tm *tm_info = localtime(&now);
char timestamp[64];
strftime(timestamp,sizeof(timestamp),"%Y-%m-%d %H:%M:%S",tm_info);
if(strcmp(level,"ERROR")==0){
    fprintf(stderr, "[%s] %s: %s\n",timestamp,level,message);
}else{
    printf("[%s] %s: %s\n",timestamp,level,message);
}
FILE *log_file = fopen(FORGE_LOG_PATH, "a");
    if (log_file == NULL) {
       printf("Warning: Could not open log file: %s\n", FORGE_LOG_PATH);
        return;
    }   
    fprintf(log_file, "[%s] %s: %s\n", timestamp, level, message);
    fclose(log_file);
}
void log_info(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args,format);
    vsnprintf(message,sizeof(message),format,args);
    va_end(args);
    write_log("INFO", message);
}
void log_warn(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args,format);
    vsnprintf(message,sizeof(message),format,args);
    va_end(args);
    write_log("WARN", message);
}
void log_error(const char *format, ...) {
    char message[256];
    va_list args;
    va_start(args,format);
    vsnprintf(message,sizeof(message),format,args);
    va_end(args);
    write_log("ERROR", message);
}
void log_debug(const char *format, ...){
    char message[256];
    va_list args;
    va_start(args,format);
    vsnprintf(message,sizeof(message),format,args);
    va_end(args);    
    write_log("DEBUG", message);
}