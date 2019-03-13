#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "sprd_fts_type.h"
#include "sprd_fts_log.h"
#include "tsx_rawdata.h"

#define ENG_RAWDATA_FILE "/mnt/vendor/productinfo/wcn/tsx_bt_data.txt"

int write_tsx_rawdata(char *data, int nLen)
{
    int rcount;
    int ret = 0, fd = -1;
    FILE * fp = NULL;
    mode_t old_mask;
    static first_flag = 1;
    if(NULL == data)
    {
        ENG_LOG("%s: req is NULL!!!",__FUNCTION__);
        ret = -1;
        return ret;
    }

    if (0 != access("/mnt/vendor/productinfo/wcn", F_OK)) {
        ret = mkdir("/mnt/vendor/productinfo/wcn", S_IRWXU | S_IRWXG | S_IRWXO);
        if (-1 == ret && (errno != EEXIST)) {
            ENG_LOG("mkdir /productinfo/wcn failed.");
            return -1;
        }
    }

    ENG_LOG("%s: %s exists",__FUNCTION__, ENG_RAWDATA_FILE);
    if(first_flag) old_mask = umask(0);
    fp = fopen(ENG_RAWDATA_FILE, "w+");
    if(first_flag) umask(old_mask);
    if(NULL == fp)
    {
        ENG_LOG("%s: fopen fail errno=%d, strerror(errno)=%s",__FUNCTION__, errno, strerror(errno));
        first_flag = 1;
        ret = -1;
        return ret;
    }
    else
    {
        first_flag = 0;
    }
    rcount = fwrite(data, sizeof(char), nLen, fp);
    ENG_LOG("%s: fwrite count %d",__FUNCTION__, rcount);
    if(nLen != rcount)
    {
        ENG_LOG("%s: rcount is not matched!",__FUNCTION__);
        ret = -1;
    }
    else
    {
        fflush(fp);
        fd = fileno(fp);
        if(fd > 0) {
            fsync(fd);
        } else {
            ENG_LOG("%s: fileno() error, strerror(errno)=%s", __FUNCTION__, strerror(errno));
            ret = -1;
        }
    }

    fclose(fp);
    return ret;
}

int read_tsx_rawdata(char *buff, int nLen)
{
  int rcount;
  int ret = 0;
  FILE * fp = NULL;
  if(NULL == buff)
  {
    ENG_LOG("%s: res is NULL!!!",__FUNCTION__);
    ret = -1;
    return ret;
  }
  if(access(ENG_RAWDATA_FILE, F_OK) == 0) {
    ENG_LOG("%s: %s exists",__FUNCTION__, ENG_RAWDATA_FILE);
    fp = fopen(ENG_RAWDATA_FILE, "r");
    if(NULL == fp)
    {
      ENG_LOG("%s: fopen fail errno=%d, strerror(errno)=%s",__FUNCTION__, errno, strerror(errno));
      ret = -1;
      return ret;
    }
    rcount = fread(buff, sizeof(char), nLen, fp);
    if(rcount <= 0)
    {
      ret = -1;
    }
    ENG_LOG("%s: fread count %d",__FUNCTION__, rcount);
    fclose(fp);
  }else{
    ret = -1;
    ENG_LOG("%s: %s not exists",__FUNCTION__, ENG_RAWDATA_FILE);
  }
  return rcount;
}

static int tsx_rawdata_rw_handler(char *buf, int len, char *rsp, int rsplen)
{
    int ret = 0;
    MSG_HEAD_T* msg_head_ptr = NULL;
    DIAG_TSX_DATA_HEAD* src_rawdata_head_ptr = NULL;
    DIAG_TSX_DATA_HEAD* dst_rawdata_head_ptr = NULL;

    if(NULL == buf || NULL == rsp){
        ENG_LOG("%s,null pointer",__FUNCTION__);
        return 0;
    }

    src_rawdata_head_ptr = (DIAG_TSX_DATA_HEAD *)(buf + 1 + sizeof(MSG_HEAD_T));
    memset(rsp, 0, rsplen);
    msg_head_ptr = (MSG_HEAD_T*)(rsp + 1);
    dst_rawdata_head_ptr = (DIAG_TSX_DATA_HEAD *)(rsp + 1 + sizeof(MSG_HEAD_T));
    memcpy(rsp, buf, sizeof(MSG_HEAD_T)+sizeof(DIAG_TSX_DATA_HEAD)+1);

    if(0 == src_rawdata_head_ptr->subcmd)
    {
        int nLen = src_rawdata_head_ptr->datalen;

        ret = write_tsx_rawdata((char*)src_rawdata_head_ptr+sizeof(DIAG_TSX_DATA_HEAD), nLen);
        ENG_LOG("status=%d ret=%d",dst_rawdata_head_ptr->status, ret);
        if(0 == ret)
        {
            dst_rawdata_head_ptr->status = 0;
        }else{
            dst_rawdata_head_ptr->status = 1;
        }
        msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(DIAG_TSX_DATA_HEAD);
        dst_rawdata_head_ptr->datalen = 0;
        ((char *)(dst_rawdata_head_ptr))[sizeof(DIAG_TSX_DATA_HEAD)] = 0x7e;
        ENG_LOG("status=%d",dst_rawdata_head_ptr->status);
    }else if(1 == src_rawdata_head_ptr->subcmd){
        char rawdata[256] = {0};
        ret = read_tsx_rawdata(rawdata, sizeof(rawdata));
        ENG_LOG("*******************ret = %d", ret);

        if( ret > 0 && ret < sizeof(rawdata))
        {
            dst_rawdata_head_ptr->status = 0;
            msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(DIAG_TSX_DATA_HEAD)+ret;
            dst_rawdata_head_ptr->datalen = ret;
            memcpy((char *)dst_rawdata_head_ptr+sizeof(DIAG_TSX_DATA_HEAD), rawdata, ret);
            ((char *)(dst_rawdata_head_ptr))[sizeof(DIAG_TSX_DATA_HEAD)+ret] = 0x7e;
        }else{
            dst_rawdata_head_ptr->status = 1;
            msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(DIAG_TSX_DATA_HEAD);
            dst_rawdata_head_ptr->datalen = 0;
            ((char *)(dst_rawdata_head_ptr))[sizeof(DIAG_TSX_DATA_HEAD)] = 0x7e;
        }
    }else{
        ENG_LOG("%s: tsx_data cmd not read and write !!!\n", __FUNCTION__);
    }

    return msg_head_ptr->len+2;
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libmiscdata");

    //1st command
    reg->type = 0x62; //main cmd 
    reg->subtype = 0x0; //sub cmd
    //reg->also_need_to_cp = 1;  //deep sleep cmd is also dealed with upon cp side
    reg->diag_ap_cmd = 0x24;
    reg->eng_diag_func = tsx_rawdata_rw_handler; // rsp function ptr
    moudles_num++;

    *num = moudles_num;
    ENG_LOG("register_this_module_ext: %d - %d",*num, moudles_num);
}
