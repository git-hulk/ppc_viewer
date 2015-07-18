/*
* ***************************************************************
 * fip_view.c is tools to view file and pagecache in process
 * author by @git-hulk at 2015-07-18 
 * Copyright (C) 2015 Inc.
 * *************************************************************

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define C_RED "\033[31m"
#define C_GREEN "\033[32m"
#define C_YELLOW "\033[33m"
#define C_PURPLE "\033[35m"
#define C_NONE "\033[0m"

#define PAGE_IN_MEM(c) ((c) & 0x1)
#define PAGE_ALIGNED(addr) ((((long)addr) & (page_size - 1))== 0)
#define PAGE_NUM(filesize) (((filesize)+page_size-1)/page_size) 

struct option {
    int is_detail;
    int just_list_file;
    char *regular;
};

struct global_stats {
    uint64_t total_pages;
    uint64_t mem_pages;
    uint64_t total_files;
    uint64_t skip_files; 
};

enum LEVEL {
    DEBUG,
    INFO,
    WARN,
    ERROR
}; 

static int page_size = 0;
static struct global_stats g_stats;
static struct option opt; 

void logger(enum LEVEL loglevel,char *fmt, ...) {
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    char *msg = NULL;
    const char *color = "";
    switch(loglevel) {
      case DEBUG: msg = "DEBUG"; break;
      case INFO: msg = "INFO"; color = C_YELLOW ; break;
      case WARN: msg = "WARN"; color= C_PURPLE; break;
      case ERROR: msg = "ERROR"; color = C_RED; break;
    }
    fprintf(stderr, "%s%s: %s\n"C_NONE, color, msg, buf);
    if(loglevel >= ERROR) {
        exit(1);
    }
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

int check_file(char *fname, int *file_size) {
    if(fname[0] != '/') {
        return 0;
    }
   
    if(opt.regular && strncmp(opt.regular, fname, strlen(opt.regular)) != 0) {
        return 0;
    }
    struct stat sb;
    stat(fname, &sb);
    if(!S_ISREG(sb.st_mode)) {
        return 0;
    }

    *file_size = sb.st_size;
    return 1;
}

void print_summary() {
    char tp_buf[128];
    char mp_buf[128];
    bytes_to_human(tp_buf,g_stats.total_pages*4096), 
    bytes_to_human(mp_buf,g_stats.mem_pages*4096), 
    fprintf(stderr, C_GREEN "========================== SUMMARY ==========================\n");
    fprintf(stderr, 
        "total_size: %s\n" \
        "mem_page_size: %s\n" \
        "total_pages: %"PRIu64"\n" \
        "mem_pages: %"PRIu64"\n" \
        "ratio: %.3f%%\n" \
        "total_files: %"PRIu64"\n" \
        "skip_files: %"PRIu64"\n" \
        "",
        tp_buf,
        mp_buf,
        g_stats.total_pages,
        g_stats.mem_pages,
        (1.0*g_stats.mem_pages)/g_stats.total_pages*100,
        g_stats.total_files,
        g_stats.skip_files
    );
    fprintf(stderr, "========================== SUMMARY ==========================\n"C_NONE);
}

void touch(char *fname) {
    int file_size = 0;
    if(!check_file(fname, &file_size)) return;
    if(opt.just_list_file)  {
        char buf[128];
        bytes_to_human(buf, file_size);        
        fprintf(stderr, "filename: %s\tfilesize:%s\n", fname, buf);
        return;
    }
    if(file_size <= 0) {
        return;
    }

    int fd, npages;
    char *mmap_addr;
    fd = open(fname, O_RDONLY, 0);
    if (fd == -1) {
        logger(INFO, "open file %s error, skip..", fname);
        return;
    }
    mmap_addr = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mmap_addr == MAP_FAILED || !PAGE_ALIGNED(mmap_addr)) {
        if(mmap_addr == MAP_FAILED) {
            logger(INFO, "mmap file %s failed, skip..", fname);
        } else {
            logger(INFO, "mmap address is not page aligned.");
        }
        close(fd);
        return;
    }
    npages = PAGE_NUM(file_size);
    g_stats.total_pages += npages;
     
    unsigned char mincore_vec[npages];
    mincore(mmap_addr, file_size, mincore_vec);
    int i, pages_in_mem = 0;
    for(i = 0; i < npages; i++) {
        if(PAGE_IN_MEM(mincore_vec[i])) {
            pages_in_mem++;
        }
    }
    if(opt.is_detail) {
        fprintf(stderr, "%s\t page_in_mem:%d\ttotal_pages:%d\n", fname, pages_in_mem, npages);
    }
    g_stats.mem_pages += pages_in_mem;
    g_stats.total_files++;

    if(munmap(mmap_addr, file_size) != 0) {
        logger(INFO, "file %s munmap failed.", fname);
    }
    close(fd);
}

// traverse /proc/{pid}/fd/* to get regular file list.
void traverse_porcess(int pid) {
    if(!pid) return;

    char buf[128];
    snprintf(buf, sizeof(buf), "/proc/%d/fd/", pid);

    if(access(buf, F_OK) != 0) {
        logger(ERROR, "maybe process %d is not exists.", pid);
    }
    if(access(buf, R_OK) != 0) {
        logger(ERROR, "access pid %d permission denied.", pid);
    }

    DIR *proc_dir = opendir(buf);
    if(!proc_dir) {
        logger(ERROR, "open %s for read failed.", buf);
    }

    struct dirent *de;
    while((de = readdir(proc_dir)) != NULL) {
        if(strcmp(de->d_name, ".") == 0 ||
            strcmp(de->d_name, "..") == 0) {
            continue;
        }
        // NOTE: file in /proc/{pid}/fd/* should be link.
        char fname[2048];
        int fname_size;
        if(DT_LNK == de->d_type) {
            memset(buf, '\0', sizeof(buf));
            memset(fname, '\0', sizeof(fname));
            snprintf(buf, sizeof(buf), "/proc/%d/fd/%s", pid, de->d_name);
            fname_size = readlink(buf, fname, sizeof(fname));
            if(fname_size == sizeof(fname)) {
                logger(INFO, "may be filename is too long.");
                continue;
            }
            touch(fname);
        }
    }

    if(!opt.just_list_file) {
        print_summary();
    }
    closedir(proc_dir);
}

static void usage() {
    printf("\n============== FIP_VIEWER USAGE ==============\n");
    printf("FIP_VIEWER is process file/pagecache finder.\n");
    printf("  -h show usage.\n");
    printf("  -p pid.\n");
    printf("  -d detail mode.\n");
    printf("  -l just list files.\n");
    printf("============== FIP_VIEWER USAGE ==============\n");
    exit(1);
}

int main(int argc, char **argv) {
    char ch;
    int is_usage = 0;
    int pid = 0;

    while((ch = getopt(argc, argv, "p:r:dlh")) != -1) {
        switch(ch) {
          case 'p': pid = atoi(optarg); break;
          case 'h': is_usage = 1; break;
          case 'd': opt.is_detail = 1; break;
          case 'l': opt.just_list_file = 1; break;
          case 'r': opt.regular = strdup(optarg); break;
        }
    }
    if(is_usage || !pid) usage();

    page_size = sysconf(_SC_PAGESIZE);
    traverse_porcess(pid);
    free(opt.regular);
    return 0;
}
