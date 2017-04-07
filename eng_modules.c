#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "eng_modules.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define  LOG_TAG "ENGPC"
#include <cutils/log.h>

#define	MAX_LEN 50

typedef void (*REGISTER_FUNC)(struct eng_callback *register_callback);

static const char *eng_modules_path = "/system/lib/engpc";

eng_modules* get_eng_modules(struct eng_callback p)
{
    ALOGD("%s",__FUNCTION__);
    eng_modules *modules = (eng_modules*)malloc(sizeof(eng_modules));
    if (modules == NULL)
    {
        ALOGE("%s malloc fail...",__FUNCTION__);
        return NULL;
    }
    memset(modules,0,sizeof(eng_modules));
    modules->callback.diag_ap_cmd = p.diag_ap_cmd;
    sprintf(modules->callback.at_cmd, "%s", p.at_cmd);
    modules->callback.eng_diag_func = p.eng_diag_func;
    modules->callback.eng_linuxcmd_func = p.eng_linuxcmd_func;

    return modules;
}

int readFileList(const char *basePath, char **f_name)
{
    DIR *dir;
    struct dirent *ptr;
    int num = 0;
    ALOGD("%s",__FUNCTION__);

    if ((dir = opendir(basePath)) == NULL)
    {
        ALOGE("Open %s error...%s",basePath,dlerror());
        return 0;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if(ptr->d_type == 8){    ///file
            ALOGD("d_name:%s/%s\n",basePath,ptr->d_name);
            f_name[num] = ptr->d_name;
            num ++;
            ALOGD("d_name:%s\n",*f_name);
        }
    }
    closedir(dir);
    return num;
}


int eng_modules_load(struct list_head *head )
{
    REGISTER_FUNC eng_register_func = NULL;
    struct eng_callback register_callback;
    char path[MAX_LEN]=" ";

    void *handler[MAX_LEN];
    char *f_name[MAX_LEN];
    char **p;
    int i = 0;
    p = f_name;
    eng_modules *modules;

    ALOGD("%s",__FUNCTION__);

    INIT_LIST_HEAD(head);
    int num = readFileList(eng_modules_path,p);
    ALOGD("file num: %d\n",num);

    for (i = 0 ; i < num; i++) {
        snprintf(path, sizeof(path), "%s/%s",
                        eng_modules_path, f_name[i]);
        ALOGD("find lib path: %s",path);

        if (access(path, R_OK) == 0){
            handler[i] = dlopen(path,RTLD_LAZY);
            if (handler[i] == NULL){
                ALOGE("%s dlopen fail! %s \n",path,dlerror());
            }else{
                eng_register_func = (REGISTER_FUNC)dlsym(handler[i], "register_this_module");
                if(!eng_register_func){
                    dlclose(handler[i]);
                    ALOGE("%s dlsym fail! %s\n",path,dlerror());
                    continue;
                }
                eng_register_func(&register_callback);

                modules = get_eng_modules(register_callback);
                if (modules == NULL){
                    continue;
                }
                list_add_tail(&modules->node, head);
            }
        }
    }
    return 0;
}

