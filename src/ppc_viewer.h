#ifndef _PPC_VIEWER_H_
#define _PPC_VIEWER_H_

#include <stdint.h>
#include "util.h"
struct option {
    int is_detail;
    int just_list_file;
    int interval;
    int count;
    enum LEVEL log_level;
    char *log_file;
    char *regular;
    char *output_file;
};

struct global_stats {
    uint64_t total_pages;
    uint64_t mem_pages;
    uint64_t total_files;
    uint64_t skip_files; 
};

void traverse_porcess(int pid);
#endif

