/*
 * ***************************************************************
 * ppc_view.h
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
    char *path;
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
void traverse_path(char *path, int flag);
#endif

