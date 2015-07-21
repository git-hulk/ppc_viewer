#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ppc_viewer.h"
#include "util.h"

struct option opt;

void logger(enum LEVEL loglevel,char *fmt, ...)
{
    FILE *fp;
    va_list ap;
    time_t now;
    char buf[4096];
    char t_buf[64];
    char *msg = NULL;
    const char *color = "";

    if(loglevel < opt.log_level) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    switch(loglevel) {
      case DEBUG: msg = "DEBUG"; break;
      case INFO:  msg = "INFO";  color = C_YELLOW ; break;
      case WARN:  msg = "WARN";  color = C_PURPLE; break;
      case ERROR: msg = "ERROR"; color = C_RED; break;
    }

    now = time(NULL);
    strftime(t_buf,64,"%Y-%m-%d %H:%M:%S",localtime(&now));
    fp = (opt.log_file == NULL) ? stdout : fopen(opt.log_file,"a");
    if(opt.log_file) {
        fprintf(fp, "[%s] [%s] %s\n", t_buf, msg, buf);
        fclose(fp);
    } else {
        fprintf(fp, "%s[%s] [%s] %s\n"C_NONE, color, t_buf, msg, buf);
    }

    if(loglevel >= ERROR) {
        exit(1);
    }
}

int check_file(char *fname) {
    if(fname[0] != '/') {
        return 0;
    }
    if(opt.regular && strncmp(opt.regular, fname, strlen(opt.regular)) != 0) {
        return 0;
    }
    return 1;
}

void bytes_to_human(char *s, unsigned long long n) {
    double d;
    if (n < 1024) {
        sprintf(s,"%lluB",n);
        return;
    } else if (n < (1024*1024)) {
        d = (double)n/(1024);
        sprintf(s,"%.2fK",d);
    } else if (n < (1024LL*1024*1024)) {
        d = (double)n/(1024*1024);
        sprintf(s,"%.2fM",d);
    } else if (n < (1024LL*1024*1024*1024)) {
        d = (double)n/(1024LL*1024*1024);
        sprintf(s,"%.2fG",d);
    } else {
        d = (double)n/(1024LL*1024*1024*1024);
        sprintf(s,"%.2fT",d);
    }
}
