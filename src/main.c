#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "ppc_viewer.h"

extern struct option opt; 
static void usage() {
    fprintf(stderr, "\n============== PPC_VIEWER USAGE ==============\n");
    fprintf(stderr, "PPC_VIEWER is process file/pagecache finder.\n");
    fprintf(stderr, "  -h show usage.\n");
    fprintf(stderr, "  -p pid.\n");
    fprintf(stderr, "  -t directory.\n");
    fprintf(stderr, "  -i interval.\n");
    fprintf(stderr, "  -c count.\n");
    fprintf(stderr, "  -d detail mode.\n");
    fprintf(stderr, "  -l just list files.\n");
    fprintf(stderr, "  -o result output file path.\n");
    fprintf(stderr, "  -f log file path\n");
    fprintf(stderr, "  -e loglevel, 1:DEBUG, 2:INFO 3:WARN 4:ERROR\n");
    fprintf(stderr, "============== PPC_VIEWER USAGE ==============\n");
    exit(1);
}

int main(int argc, char **argv) {
    char ch;
    int i;
    int is_usage = 0;
    int pid = 0;

    while((ch = getopt(argc, argv, "p:r:t:dlhi:c:f:e:o:")) != -1) {
        switch(ch) {
            case 'p': pid = atoi(optarg); break;
            case 't': opt.path = strdup(optarg); break;
            case 'i': opt.interval = atoi(optarg); break;
            case 'c': opt.count = atoi(optarg); break;
            case 'h': is_usage = 1; break;
            case 'd': opt.is_detail = 1; break;
            case 'l': opt.just_list_file = 1; break;
            case 'r': opt.regular = strdup(optarg); break;
            case 'f': opt.log_file = strdup(optarg); break;
            case 'e': opt.log_level = atoi(optarg); break;
            case 'o': opt.output_file = strdup(optarg); break;
        }
    }
    if(is_usage || ( !pid && opt.path == NULL )) {
        usage();
    }
    else if ( pid && opt.path != NULL) {
        usage();
    }

    if(opt.interval <= 0) {
        opt.interval = 0;
    }
    if(opt.count <= 0) {
        opt.count = 1;
    }
    if(opt.log_level <= 0 ) {
        opt.log_level = INFO;
    }

    if ( pid ) {
        traverse_porcess(pid);
        if(opt.interval > 0) {
            for(i = 1; i < opt.count; i++) {
                sleep(opt.interval);
                traverse_porcess(pid);
            }
        }
    }
    else {
        traverse_path(opt.path, 1);
        if (opt.interval > 0) {
            for (i = 1; i < opt.count; i++) {
                sleep(opt.interval);
                traverse_path(opt.path, 1);
            }
        }
    }

    free(opt.regular);
    free(opt.log_file);
    free(opt.output_file);
    free(opt.path);
    return 0;
}
