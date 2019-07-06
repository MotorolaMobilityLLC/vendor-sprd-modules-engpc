#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "CModuleMgr.h"
#include "../common/englog.h"
#include "modules.h"
#include "bqb.h"

#define ENG_LOG EngLog::info

extern CModuleMgr* g_lpModMgr;

#define MAX_CHNL_BUFF  (64*1024)

int write_to_host_diag(char* buff, int len){
    if (g_lpModMgr != NULL && g_lpModMgr->m_lpHostDiagPort != NULL){
        return g_lpModMgr->m_lpHostDiagPort->write(buff, len);
    }else{
        ENG_LOG("write_to_host_diag fail: g_lpModMgr = NULL or s_lpModMgr->m_lpHostDiagPort = NULL");
    }

    return 0;
}

int chnl_send_diag_interface(char* buff, int len){
    char rsp[MAX_CHNL_BUFF] = {0};
    int cp_process = 0;
    if (g_lpModMgr != NULL){
        return g_lpModMgr->processDiag(DATA_DIAG, buff, len, rsp, sizeof(rsp), cp_process);
    }

    return 0;
}

int chnl_send_at_interface(char* buff, int len){
    char rsp[MAX_CHNL_BUFF] = {0};
    int cp_process = 0;
    if (g_lpModMgr != NULL){
        return g_lpModMgr->processAT(DATA_AT, buff, len, rsp, sizeof(rsp), cp_process);
    }

    return 0;
}

DYMIC_WRITETOPC_FUNC write_interface[WRITE_TO_MAX] = {
    write_to_host_diag,/*pc lte diag*/
    NULL,NULL,NULL,NULL,NULL,
    chnl_send_diag_interface,
    NULL,
    chnl_send_at_interface,/*send cmd so to so*/
    NULL,NULL,NULL,NULL,NULL,NULL
};

static const char *VENDOR_LIBRARY_NAME = "libbqbbt.so";
static const char *VENDOR_LIBRARY_SYMBOL_NAME = "BLUETOOTH_BQB_INTERFACE";

void* CModuleMgr::m_libHdl = NULL;
void* CModuleMgr::m_libInterface = NULL;

CModuleMgr::CModuleMgr(char* dir){
    memcpy(m_path, dir, strlen(dir));
    m_lpHostDiagPort = NULL;
    m_isLoaded = false;
}

CModuleMgr::~CModuleMgr(){
}

int CModuleMgr::load(){
    int ret = eng_modules_load();
    m_isLoaded = (ret==0)?true:false;
    return ret;
}

int CModuleMgr::process(DATA_TYPE type, char *buf, int len, char *rsp, int rsp_len, int& cp_process){
    if (!m_isLoaded){
        load();
    }

    if (type == DATA_AT)
        return processAT(type, buf, len, rsp, rsp_len, cp_process);
    if (type == DATA_WCN_AT)
        return processWcnAT(type, buf, len, rsp, rsp_len, cp_process);
    if (type == DATA_DIAG)
        return processDiag(type, buf, len, rsp, rsp_len, cp_process);
    if (type == DATA_AGDSP_LOG||type == DATA_AGDSP_LOG||type == DATA_AGDSP_LOG||type == DATA_AGDSP_LOG)
        return processSmp(type, buf, len, rsp, rsp_len, cp_process);

    return 0;
}

int CModuleMgr::processDiag(DATA_TYPE type, char *buf, int len, char *rsp, int rsp_len, int& cp_process){
    eng_modules *modules_list = NULL;
    struct list_head *list_find = NULL;
    MSG_HEAD_T *msg_head_ptr = (MSG_HEAD_T *)(buf + 1);
    int rlen = 0;
    unsigned int *data_cmd = NULL;
    byte *data = NULL;
    unsigned short *apcmd = NULL;

    list_for_each(list_find,&m_listHead){
        modules_list = list_entry(list_find, eng_modules, node);

        if ((buf[7] == 0x68) && (0 != strlen(modules_list->callback.at_cmd)) && (strcasestr(buf+9, modules_list->callback.at_cmd)) != NULL) { // at command
            ENG_LOG("%s: Dymic CMD=%s finded\n",__FUNCTION__,modules_list->callback.at_cmd);
            if (NULL != modules_list->callback.eng_linuxcmd_func) {
                rlen = modules_list->callback.eng_linuxcmd_func(buf, rsp);

                //for case :need to ap & cp
                if (modules_list->callback.also_need_to_cp) {
                    cp_process = 1;
                }

                break;
            } else {
                ENG_LOG("%s: Dymic eng_linuxcmd_func == NULL\n",__FUNCTION__);
                break;
            }
        } else if ( (msg_head_ptr->type == modules_list->callback.type && msg_head_ptr->subtype == modules_list->callback.subtype) ||
                    (modules_list->callback.eng_cmd_match != NULL && modules_list->callback.eng_cmd_match(buf, len) == 0) )  {
            // diag command: type(unsigned char) + sub_type(unsigned char) + data_cmd(unsigned int)
            if (0x5D == msg_head_ptr->type){
                data_cmd = (unsigned int *)(buf + 1 + sizeof(MSG_HEAD_T));
                if (*data_cmd != modules_list->callback.diag_ap_cmd) {
                    ENG_LOG("%s data cmd is not matched!", __FUNCTION__);
                    continue;
                }
            }

            // diag command: type(unsigned char) + sub_type(unsigned char) + TOOLS_DIAG_AP_CMD_T
            if (0x62 == msg_head_ptr->type){//DIAG_CMD_APCALI
                apcmd = (unsigned short *)(buf + 1 + sizeof(MSG_HEAD_T));
                if (*apcmd != modules_list->callback.diag_ap_cmd) {
                    ENG_LOG("%s apcmd->cmd is not matched!", __FUNCTION__);
                    continue;
                }
            }

            // diag command: type(unsigned char) + sub_type(unsigned char) + byte, for autotest test case which have same type+subtype
            if (0x38 == msg_head_ptr->type){
                data = (byte *)(buf + 1 + sizeof(MSG_HEAD_T));
                if (modules_list->callback.diag_ap_cmd != -1 && (int)(*data) != modules_list->callback.diag_ap_cmd)  {
                    ENG_LOG("%s data is not matched!", __FUNCTION__);
                    continue;
                }
            }

            if (NULL != modules_list->callback.eng_diag_func) {
                if (NULL != modules_list->callback.eng_set_writeinterface_func) {
                    modules_list->callback.eng_set_writeinterface_func(write_interface);
                }
                rlen = modules_list->callback.eng_diag_func(buf, len, rsp, rsp_len);
                if (rlen == ENG_DIAG_RET_UNSUPPORT){
                    ENG_LOG("%s eng_diag_func return %d, continue find next callback func", __FUNCTION__, ENG_DIAG_RET_UNSUPPORT);
                    continue;
                }

                //for case :need to ap & cp
                if (modules_list->callback.also_need_to_cp) {
                    cp_process = 1;
                }

                break;
            } else {
                ENG_LOG("%s: Dymic eng_diag_func == NULL\n",__FUNCTION__);
                break;
            }
        } else {
            continue;
        }
    }

    return rlen;
}

int CModuleMgr::processAT(DATA_TYPE type, char *buf, int len, char *rsp, int rsp_len, int& cp_process){
    int rlen = 0, ret = 0;
    eng_modules *modules_list = NULL;
    struct list_head *list_find;

    ENG_LOG("%s: buf:%s len:%d rsp_len:%d", __FUNCTION__, buf, len, rsp_len);

    list_for_each(list_find, &m_listHead) {
        modules_list = list_entry(list_find, eng_modules, node);

        if ((0 != strlen(modules_list->callback.at_cmd)) &&
            (0 == strncmp((const char *) buf, (const char *)(modules_list->callback.at_cmd),
            strlen(modules_list->callback.at_cmd)))) { // at command
            ENG_LOG("%s: Dymic CMD=%s finded\n", __FUNCTION__, modules_list->callback.at_cmd);
            if (NULL != modules_list->callback.eng_linuxcmd_func) {
                rlen = modules_list->callback.eng_linuxcmd_func(buf, rsp);

                //for case :need to ap & cp
                if (modules_list->callback.also_need_to_cp) {
                    cp_process = 1;
                }

                break;
            } else {
              ENG_LOG("%s: Dymic eng_linuxcmd_func == NULL\n", __FUNCTION__);
              break;
            }
        }else {
          continue;
        }
    }

    return rlen;
}

int CModuleMgr::processSmp(DATA_TYPE type, char *buf, int len, char *rsp, int rsp_len, int& cp_process){
    memcpy(rsp, buf, len);
    return len;
}

int CModuleMgr::processWcnAT(DATA_TYPE type, char *buf, int len, char *rsp, int rsp_len, int& cp_process){
    int rlen = 0;

    if (m_libHdl == NULL){
        bqb_vendor_open();
    }

    if(m_libInterface != NULL ){
        bt_bqb_interface_t* pIf = (bt_bqb_interface_t *)m_libInterface;
        pIf->set_fd(m_lpHostDiagPort->getFD());
        rlen = pIf->check_received_str(m_lpHostDiagPort->getFD(), buf, len);

        if (0 == rlen && pIf->get_bqb_state() == BQB_OPENED){
            pIf->eng_send_data(buf, len);
        }
    }else{
        ENG_LOG("m_libInterface is NULL!");
    }

    return ENG_DIAG_NO_RESPONSE;
}

int CModuleMgr::bqb_vendor_open(){
    m_libHdl = dlopen(VENDOR_LIBRARY_NAME, RTLD_NOW);
    if (!m_libHdl) {
        ENG_LOG("unable to open %s: %s", VENDOR_LIBRARY_NAME, dlerror());
        goto error;
    }

    m_libInterface = (bt_bqb_interface_t *)dlsym(m_libHdl, VENDOR_LIBRARY_SYMBOL_NAME);
    if (!m_libInterface) {
        ENG_LOG("unable to find symbol %s in %s: %s", VENDOR_LIBRARY_SYMBOL_NAME, VENDOR_LIBRARY_NAME, dlerror());
        goto error;
    }
    ((bt_bqb_interface_t *)m_libInterface)->init();

    return 0;

error:
    m_libInterface = NULL;
    if (m_libHdl)
    dlclose(m_libHdl);
    m_libHdl = NULL;

    return -1;
}

eng_modules* CModuleMgr::get_eng_modules(struct eng_callback p){
    //ENG_LOG("%s",__FUNCTION__);
    eng_modules *modules = (eng_modules*)malloc(sizeof(eng_modules));
    if (modules == NULL)
    {
        ENG_LOG("%s malloc fail...",__FUNCTION__);
        return NULL;
    }
    memset(modules,0,sizeof(eng_modules));
    modules->callback.type = p.type;
    modules->callback.subtype = p.subtype;
    modules->callback.diag_ap_cmd = p.diag_ap_cmd;
    modules->callback.also_need_to_cp = p.also_need_to_cp;
    sprintf(modules->callback.at_cmd, "%s", p.at_cmd);
    modules->callback.eng_diag_func = p.eng_diag_func;
    modules->callback.eng_linuxcmd_func = p.eng_linuxcmd_func;
    modules->callback.eng_set_writeinterface_func = p.eng_set_writeinterface_func;
    modules->callback.eng_cmd_match = p.eng_cmd_match;

    return modules;
}

int CModuleMgr::eng_modules_load(){
    REGISTER_FUNC eng_register_func = NULL;
    REGISTER_EXT_FUNC eng_register_ext_func = NULL;
    struct eng_callback register_callback;
    struct eng_callback register_arr[32];
    struct eng_callback *register_arr_ptr = register_arr;
    int register_num = 0;
    int i = 0;
    char path[MAX_MODULE_FILE_PATH]=" ";
    char lnk_path[MAX_MODULE_FILE_PATH]=" ";
    int readsize = 0;

    eng_modules *modules;

    //get so name fail:empty
    DIR *dir;
    struct dirent *ptr;
    void *handler = NULL;

    ENG_LOG("%s",__FUNCTION__);

    INIT_LIST_HEAD(&m_listHead);
    if ((dir = opendir(m_path)) == NULL)
    {
        ENG_LOG("Open %s error...%s",m_path,dlerror());
        return 0;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (ptr->d_type == 8 || ptr->d_type == 10) { /// file  , 10 == DT_LNK
            ENG_LOG("d_name:%s/%s", m_path, ptr->d_name);
            snprintf(path, sizeof(path), "%s/%s", m_path, ptr->d_name);
            ENG_LOG("find lib path: %s", path);

            if (ptr->d_type == 10) //DT_LNK
            {
                memset(lnk_path,0,sizeof(lnk_path));
                readsize = readlink(path, lnk_path, sizeof(lnk_path));
                ENG_LOG("%s readsize:%d lnk_path:%s \n", path ,readsize, lnk_path);

                if(readsize == -1) {
                    ENG_LOG("ERROR! Fail to readlink!\n");
                    continue;
                }

                memset(path, 0, sizeof(path));
                strncpy(path, lnk_path, strlen(lnk_path));
            }

            if (access(path, R_OK) == 0) {
                handler = NULL;
                handler = dlopen(path, RTLD_LAZY);
                if (handler == NULL) {
                    ENG_LOG("%s dlopen fail! %s \n", path, dlerror());
                } else {
                    eng_register_func = (REGISTER_FUNC)dlsym(handler, "register_this_module");
                    if (eng_register_func != NULL) {
                        memset(&register_callback, 0, sizeof(struct eng_callback));
                        register_callback.diag_ap_cmd = -1;
                        register_callback.type = 0xFF;
                        register_callback.subtype = 0xFF;
                        eng_register_func(&register_callback);
                        ENG_LOG("%d:type:%d subtype:%d data_cmd:%d at_cmd:%s", i,
                        register_callback.type, register_callback.subtype,
                        register_callback.diag_ap_cmd, register_callback.at_cmd);
                        modules = get_eng_modules(register_callback);
                        if (modules == NULL) {
                            ENG_LOG("%s modules == NULL\n", __FUNCTION__);
                            continue;
                        }
                        list_add_tail(&modules->node, &m_listHead);
                    }

                    eng_register_ext_func = (REGISTER_EXT_FUNC)dlsym(handler, "register_this_module_ext");
                    if (eng_register_ext_func != NULL) {
                        memset(register_arr, 0, sizeof(register_arr));
                        for (i = 0; i < sizeof(register_arr)/sizeof(struct eng_callback); i++) {
                            register_arr[i].diag_ap_cmd = -1;
                            register_arr[i].type = 0xFF;
                            register_arr[i].subtype = 0xFF;
                        }
                        eng_register_ext_func(register_arr_ptr, &register_num);
                        ENG_LOG("register_num:%d",register_num);

                        for (i = 0; i < register_num; i++) {
                            ENG_LOG("%d:type:%d subtype:%d data_cmd:%d at_cmd:%s", i,
                            register_arr[i].type, register_arr[i].subtype,
                            register_arr[i].diag_ap_cmd, register_arr[i].at_cmd);
                            modules = get_eng_modules(register_arr[i]);
                            if (modules == NULL) {
                                ENG_LOG("%s modules == NULL\n", __FUNCTION__);
                                continue;
                            }
                            list_add_tail(&modules->node, &m_listHead);
                        }
                    }
                    if (eng_register_func == NULL && eng_register_ext_func == NULL) {
                        dlclose(handler);
                        ENG_LOG("%s dlsym fail! %s\n", path, dlerror());
                        continue;
                    }
                }
            } else {
                ENG_LOG("%s is not allow to read!\n", path);
            }
        }
    }

    closedir(dir);
    return 0;
}

void CModuleMgr::print(){
/*
    eng_modules *modules_list = NULL;
    struct list_head *list_find = NULL;

    list_for_each(list_find,&m_listHead){
        modules_list = list_entry(list_find, eng_modules, node);

    }
*/
}
