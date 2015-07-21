#ifndef _UTIL_H_
#define _UTIL_H_

#define C_RED "\033[31m"
#define C_GREEN "\033[32m"
#define C_YELLOW "\033[33m"
#define C_PURPLE "\033[35m"
#define C_NONE "\033[0m"

#define PAGE_IN_MEM(c) ((c) & 0x1)
#define PAGE_ALIGNED(addr) ((((long)addr) & (PAGE_SIZE - 1))== 0)
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#define PAGE_NUM(filesize) (((filesize)+PAGE_SIZE-1)/PAGE_SIZE) 

enum LEVEL {
    DEBUG = 1,
    INFO,
    WARN,
    ERROR
}; 

void logger(enum LEVEL loglevel,char *fmt, ...);
int check_file(char *fname);
void bytes_to_human(char *s, unsigned long long n); 
#endif
