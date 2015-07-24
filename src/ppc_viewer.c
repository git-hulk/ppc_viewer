/*
 * ***************************************************************
 * fip_view.c main implemention of ppc_viewer is here 
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
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ppc_viewer.h"

static struct global_stats g_stats;
struct option opt; 

static void print_summary() {
    char tp_buf[128];
    char mp_buf[128];
    double mem_ratio = 0;
    FILE *fp = stderr;

    bytes_to_human(tp_buf,g_stats.total_pages*4096); 
    bytes_to_human(mp_buf,g_stats.mem_pages*4096); 
    if(g_stats.total_pages > 0) {
        mem_ratio = (1.0*g_stats.mem_pages)/g_stats.total_pages*100;
    }

    if (opt.output_file) {
        fp = fopen(opt.output_file,"a");
    } else {
        fprintf(fp, C_GREEN);
    }
    fprintf(fp, "========================== SUMMARY ==========================\n");
    fprintf(fp, 
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
            mem_ratio,
            g_stats.total_files,
            g_stats.skip_files
           );
    fprintf(fp, "========================== SUMMARY ==========================\n");
    if(opt.output_file) {
        fclose(fp);
    } else {
        fprintf(fp, C_NONE);
    }
}

static void print_file(char *fname) {
    char *delete_str = " (deleted)";
    int f_len = strlen(fname);
    int d_len = strlen(delete_str);
    FILE *fp; 

    // 文件被删除的fd
    fp = (opt.output_file == NULL) ? stdout : fopen(opt.output_file,"a");
    if(strncmp(fname+f_len-d_len, delete_str, d_len) == 0) {
        fprintf(fp, "%s\n", fname);
        goto close_fp;
    }

    struct stat sb;
    if(stat(fname, &sb) != 0) {
        logger(WARN, "can't stat file :%s", fname);
        goto close_fp;
    }
    if(!S_ISREG(sb.st_mode)) {
        goto close_fp;
    }

    char buf[128];
    bytes_to_human(buf, sb.st_size);        
    fprintf(fp, "filename: %s\tfilesize:%s\n", fname, buf);

close_fp:
    if(opt.output_file) {
        fclose(fp);
    }
}

static uint64_t get_file_size(char *fname) {
    struct stat sb;
    if(stat(fname, &sb) != 0) {
        if ( '/' == fname[0]) {
            g_stats.skip_files++; 
            logger(WARN, "can't stat file %s", fname); 
        }
        return 0;
    }
    if(!S_ISREG(sb.st_mode)) {
        g_stats.skip_files++; 
        logger(DEBUG, "not regular file %s", fname); 
        return 0;
    }
    if(sb.st_size <= 0) {
        g_stats.skip_files++; 
        logger(INFO, "empty file %s, skip..", fname);
        return 0;
    }
    return sb.st_size;
}

static void touch(char *fname) {
    if(!check_file(fname)) return;

    uint64_t file_size;
    if((file_size = get_file_size(fname)) == 0) {
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
        g_stats.skip_files++; 
        close(fd);
        return;
    }

    npages = PAGE_NUM(file_size);
    unsigned char mincore_vec[npages];
    mincore(mmap_addr, file_size, mincore_vec);
    int i, pages_in_mem = 0;
    for(i = 0; i < npages; i++) {
        if(PAGE_IN_MEM(mincore_vec[i])) {
            pages_in_mem++;
        }
    }

    if(opt.is_detail) {
        FILE *fp;
        fp = (opt.output_file == NULL) ? stdout : fopen(opt.output_file,"a");
        fprintf(fp, "%s\t page_in_mem:%d\ttotal_pages:%d\n", fname, pages_in_mem, npages);
        if(opt.output_file) {
            fclose(fp);
        }
    }

    g_stats.total_files++;
    g_stats.total_pages += npages;
    g_stats.mem_pages += pages_in_mem;

    if(munmap(mmap_addr, file_size) != 0) {
        logger(INFO, "file %s munmap failed.", fname);
    }
    close(fd);
}


void handle_link(char *path){
    char fname[2048];
    int fname_size;
    memset(fname, '\0', sizeof(fname));
    fname_size = readlink(path, fname, sizeof(fname));
    if(fname_size == -1) {
        logger(WARN, "readlink error, file: %s", path);
        return;
    }
    if(fname_size == sizeof(fname)) {
        logger(INFO, "may be filename is too long");
        return;
    }
    if(!check_file(fname)) {
        return;
    }

    if(opt.just_list_file) {
        print_file(fname);
    } 
    else 
    {
        touch(fname);
    }
}

// traverse /proc/{pid}/fd/* to get regular file list.
void traverse_porcess(int pid) {
    // reset stats
    memset(&g_stats, '\0', sizeof(struct global_stats));

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
        char dirent_path[2048];
        if(DT_LNK == de->d_type) {
            memset(dirent_path, '\0', sizeof(dirent_path));
            snprintf(dirent_path, sizeof(dirent_path), "/proc/%d/fd/%s", pid, de->d_name);
            handle_link(dirent_path);
        }
    }

    if(!opt.just_list_file) {
        print_summary();
    }
    closedir(proc_dir);
}

void traverse_path(char *path, int flag) {

    if (flag){
        memset(&g_stats, '\0', sizeof(struct global_stats));
        if(access(path, F_OK) != 0) {
            logger(ERROR, "maybe this file or directory is not exists: %s .", path );
        }
        if(access(path, R_OK) != 0) {
            logger(ERROR, "access this file or directory permission denied :%s .", path);
        }
    }

    struct stat sb;
    if (lstat(path, &sb) != 0){
        logger(WARN, "can't stat this file or directory: %s .", path);
    }
    if (S_ISLNK(sb.st_mode)){
        handle_link(path);
    }
    else if(S_ISREG(sb.st_mode)){
        if(opt.just_list_file) {
            print_file(path);
        } 
        else {
            touch(path);
        }
    }
    else if (S_ISDIR(sb.st_mode)){

        DIR *proc_dir = opendir(path);
        if(!proc_dir) {
            logger(ERROR, "open %s for read failed.", path);
        }

        struct dirent *de;
        while((de = readdir(proc_dir)) != NULL) {
            if(strcmp(de->d_name, ".") == 0 ||
                    strcmp(de->d_name, "..") == 0) {
                continue;
            }
            char dirent_path[2048];
            memset(dirent_path, '\0', sizeof(dirent_path));
            snprintf(dirent_path, sizeof(dirent_path), "%s/%s", path, de->d_name);
            traverse_path(dirent_path, 0);
        }
        closedir(proc_dir);
    }

    if(flag && !opt.just_list_file) {
        print_summary();
    }
}
