/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ENGPC_CMD_PARSE_H_
#define _ENGPC_CMD_PARSE_H_

#define MAX_LINE_LEN 256
#define ENG_CMD_SUPPORT_CONFIG "/vendor/etc/ENG_CMD.conf"
#define ENG_CONFIG_TYPE_NAME "eng_cmd"

#define MAX_SUPPORT_CMDS 256
#define BUF_LEN 256

typedef struct struct_support_cmds{
  char cmd_original[BUF_LEN];
  char cmd_customer[BUF_LEN];
  int supported;
}struct_support_cmds;

static struct_support_cmds support_cmds[MAX_SUPPORT_CMDS];

int parse_config(void);
int parse_support_cmd(int argc, char *argv[],char *ouput_cmd);

#endif