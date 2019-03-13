/**
 * Copyright (C) 2016 Spreadtrum Communications Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <poll.h>
#include <string.h>

#include "engpc_cmd.h"
#include "engpc_cmd_parser.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

static int CONFIG_CMDS_ARRAY_COUNT = 0;

static int parse_string(char * buf, char gap, int* value)
{
    int len = 0;
    char *ch = NULL;
    char str[10] = {0};

    if(buf != NULL && value  != NULL){
        ch = strchr(buf, gap);
        if(ch != NULL){
            len = ch - buf ;
            strncpy(str, buf, len);
            *value = atoi(str);
        }
    }

    return len;
}

char *parse_string_ptr(char *src, char c)
{
    char *results;
    results = strchr(src, c);
    if(results == NULL) {
        LOGD("%s is null!", results);
        return NULL;
    }
    results++ ;
    while(results[0]== c)
        results++;
    return results;
}

char * parse_string_get(char *buf, char c, char *str)
{
    int len = 0;
    char *ch = NULL;

    if(buf != NULL){
        ch = strchr(buf, c);
        if(ch != NULL){
            len = ch - buf ;
            strncpy(str, buf, len);
        }
    }
    return str;
}

int parse_entries(char *buf,char *cmd_original,char *cmd_customer, char delims)
{
    char *pos1, *pos2;
    int supported = 0;

    /* fetch each field */
    if((pos1 = parse_string_ptr(buf, delims)) == NULL)
        return -1;
    else parse_string_get(pos1, delims,cmd_original);

    if((pos2 = parse_string_ptr(pos1, delims)) == NULL)
        return -1;
    else  parse_string_get(pos2, '\n',cmd_customer);
    return 0;
}

int parse_cmd_delims(char* input_cmd,char *outputcmd, char *outputval,char *delims)
{
    char *p;
    char *buffer;
    int count = 0;
    int cmd_len = 0;
    buffer = strdup(input_cmd);
    count = strlen(buffer) + 1;
    LOGD("%s: buffer=%s",__FUNCTION__, buffer);
    p = strtok(buffer,delims);
    if(p != NULL){
        cmd_len = strlen(p) + 1;
        snprintf(outputcmd, cmd_len, "%s", p);
    }
    if(buffer != NULL){
        LOGD("%s: buffer count=%d,cmd_len=%d",__FUNCTION__, count,cmd_len);
        strncpy(outputval, buffer + cmd_len, count - cmd_len);
    }
    /*while(p!=NULL){
        LOGD("%s: word=%s",__FUNCTION__, p);
        p = strtok(NULL,delims);
        count ++;
    }*/
    LOGD("%s: outputcmd=%s,outputval=%s",__FUNCTION__, outputcmd,outputval);
    return 0;
}

int parse_support_cmd(int argc, char *argv[],char *output_cmd)
{
    LOGD("%s: argc=%d", __FUNCTION__,argc);
    if(argv == NULL){
        return 0;
    }
    //char* input_cmd
    int i = 0;
    int count = 0;
    char buffer[BUF_LEN] = {0};
    char val_buffer[BUF_LEN] = {0};
    char *delims={ "=" };
    count = CONFIG_CMDS_ARRAY_COUNT;
    LOGD("%s: count=%d,support_cmds=%d", __FUNCTION__,count,sizeof(support_cmds));

    parse_cmd_delims(argv[1],buffer,val_buffer,delims);

    LOGD("%s: parse buffer=%s,val_buffer=%s,strlen(buffer)=%d", __FUNCTION__,buffer,val_buffer,strlen(buffer));
    for(i = 0;i < count; i++){
        if(support_cmds[i].supported && !strncmp(buffer, support_cmds[i].cmd_customer, strlen(buffer) + 1)){
           LOGD("%s: This CMD is supported by default!", __FUNCTION__);
           if(argc > 2){
                snprintf(output_cmd, strlen(support_cmds[i].cmd_original) + strlen(argv[2]) + 1 + 1, "%s=%s", support_cmds[i].cmd_original,argv[2]);
           }else{
                snprintf(output_cmd, strlen(support_cmds[i].cmd_original) + strlen(val_buffer) + 1 + 1, "%s=%s", support_cmds[i].cmd_original,val_buffer);
           }
           LOGD("mmitest parse output_cmd=%s",output_cmd);
           return 1;
        }
    }
    return 0;
}

int parse_config()
{
    FILE *fp;
    int ret = 0, count = 0, err = 0;
    int i = 0;
    char buffer[MAX_LINE_LEN]={0};
    char cmd_original[BUF_LEN] = {0};
    char cmd_customer[BUF_LEN] = {0};
    int supported = 0;

    fp = fopen(ENG_CMD_SUPPORT_CONFIG, "r");
    if(fp == NULL) {
        LOGE("mmitest open %s failed! %d IN", ENG_CMD_SUPPORT_CONFIG, __LINE__);
        return -1;
    }
    /* parse line by line */
    ret = 0;
    i = 0;
    CONFIG_CMDS_ARRAY_COUNT = 0;
    while(fgets(buffer, MAX_LINE_LEN, fp) != NULL) {
        if('#'==buffer[0])
            continue;
        if(!strncmp(ENG_CONFIG_TYPE_NAME, buffer, strlen(ENG_CONFIG_TYPE_NAME))){
            LOGD("mmitest parse %s",ENG_CONFIG_TYPE_NAME);
            memset(cmd_original,0,sizeof(cmd_original));
            memset(cmd_customer,0,sizeof(cmd_customer));
            ret = parse_entries(buffer,cmd_original,cmd_customer ,'\t');
            if(ret != 0){
              LOGD("mmitest parse again by ' '!%s",ENG_CONFIG_TYPE_NAME);
              ret = parse_entries(buffer,cmd_original,cmd_customer ,' ');
            }
            if(ret != 0) {
                LOGD("mmitest parse %s,buffer=%s return %d.  reload",ENG_CMD_SUPPORT_CONFIG, buffer,ret);
                fclose(fp);
                return -1;
            }
            snprintf(support_cmds[i].cmd_original, strlen(cmd_original) + 1, "%s", cmd_original);
            snprintf(support_cmds[i].cmd_customer, strlen(cmd_customer) + 1, "%s", cmd_customer);
            //support_cmds[i].cmd_original = cmd_original;
            //support_cmds[i].cmd_customer = cmd_original;
            if(strlen(cmd_original) > 0){
                supported = 1;
            }
            support_cmds[i].supported = supported;
            LOGD("mmitest parse cmd_original=%s,cmd_customer=%s,supported=%d",cmd_original,cmd_customer,supported);
            LOGD("mmitest parse support_cmds cmd_original=%s,cmd_customer=%s,i=%d",support_cmds[i].cmd_original,support_cmds[i].cmd_customer,i);
            i ++;
            CONFIG_CMDS_ARRAY_COUNT = CONFIG_CMDS_ARRAY_COUNT + 1;
        }else{
            //ignore
        }
    }

    fclose(fp);
    return ret;
}


