/*
* ***************************************************************
 * util.h 
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
