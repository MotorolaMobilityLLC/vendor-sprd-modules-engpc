#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <linux/fs.h>

#include "sprd_fts_type.h"
#include "sprd_fts_diag.h"
#include "sprd_fts_log.h"
#include "partinfo_rw.h"
#include "cutils/properties.h"


#define RO_MODEM_FIXNVSIZE "ro.vendor.modem.fixnv_size"
#define PARTINFO_PATH "/dev/block/by-name/"


static DYMIC_WRITETOPC_FUNC g_func[WRITE_TO_MAX] = {NULL};

int get_fw_ptr(DYMIC_WRITETOPC_FUNC * write_interface_ptr){
    ENG_LOG("%s", __FUNCTION__);

    for(int i = 0; i < WRITE_TO_MAX; i++) {
        g_func[i] = write_interface_ptr[i];
        if(g_func[i] != NULL)
            ENG_LOG("%s ad 0x%x, i %d", __FUNCTION__, g_func[i], i);
    }

    return 0;
}

int findpart(char *basePath, char* partname, char **foundname)
{
    DIR *dir;
    struct dirent *ptr;
    int found = 0;
    ENG_LOG("%s %s%s", __func__, basePath, partname);

    if ((dir=opendir(basePath)) == NULL) {
        ENG_LOG("Open dir:%s error... errno=%d", basePath, errno);
        return 0;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;

        if(ptr->d_type == 10) {
            ///link file
            ENG_LOG("d_name:%s%s\n",basePath, ptr->d_name);
            char *ret = strstr(ptr->d_name, partname);
            if(ret != NULL) {
                found++;
                *foundname = (char*)malloc(strlen(ptr->d_name) + 1);
                strcpy(*foundname, ptr->d_name);
                ENG_LOG("found:%s%s\n",basePath, *foundname);
                break;
            }
        }

    }
    closedir(dir);
    ENG_LOG("%s found=%d", __func__, found);
    return found;
}

int get_match(char *basePath, char* partname, char **foundname)
{
    DIR *dir;
    struct dirent *ptr;
    int found = 0;

    ENG_LOG("%s %s%s", __func__, basePath, partname);

    dir = opendir(basePath);
    if ( dir == NULL) {
        ENG_LOG("Open dir:%s error...", basePath);
        return 0;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;

        if(ptr->d_type == 10) {
            ///link file
            printf("d_name:%s/%s\n",basePath,ptr->d_name);
            char *ret = strstr(ptr->d_name, partname);
            if(ret != NULL) {
                foundname[found] = (char*)malloc(strlen(ptr->d_name)+1);
                strcpy(foundname[found], ptr->d_name);
                found++;
                if(found > 4) {
                    ENG_LOG("%s found too much", __func__);
                    break;
                }
            }
        }
    }
    closedir(dir);
    ENG_LOG("%s found=%d", __func__, found);
    return found;
}

int partinfo_query(char *buf, int len, char *rsp, int rsplen){

    char *real_partname = NULL;
    int part_size=0;

    MSG_HEAD_T	*msg_head_ptr = NULL;
    diag_query_partition_info_req_t *lpDiagQuery = NULL;
    diag_query_partition_info_ack_t *lpDiagAck = NULL;
    ENG_LOG("%s", __FUNCTION__);
    memcpy(rsp, buf, sizeof(MSG_HEAD_T)+1);
    msg_head_ptr = (MSG_HEAD_T*)(rsp+1);
    lpDiagAck = (diag_query_partition_info_ack_t*)(rsp+sizeof(MSG_HEAD_T)+1);
    lpDiagAck->cmd = 0;
    msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(diag_query_partition_info_ack_t);

    lpDiagQuery = (diag_query_partition_info_req_t* )(buf + 1 + sizeof(MSG_HEAD_T));

    if(findpart(PARTINFO_PATH, lpDiagQuery->partition_name, &real_partname)) {
        lpDiagAck->status = 0; //success
	int namelen = 0;

        namelen = strlen(real_partname)+strlen(PARTINFO_PATH);
        char *fullpath = (char*)malloc(namelen + 1);
        if(!fullpath) {
            ENG_LOG("the %s malloc fail\n", __FUNCTION__);
            return 0;
        }
        memset(fullpath, 0, namelen);
        sprintf(fullpath, "%s%s", PARTINFO_PATH, real_partname);
        int fd = open(fullpath,O_RDONLY);
        if(fd < 0) {
            ENG_LOG("the %s--- open %s path fail\n", __FUNCTION__, fullpath);
            goto out;
        }

        int ret = ioctl(fd, BLKGETSIZE64, &part_size);
        if(ret < 0) {
            ENG_LOG("the %s--- ioctl ret= %d fail\n", __FUNCTION__, ret);
            goto out;
        }

        if(strcmp(lpDiagQuery->partition_name, "fixnv")==0) {
            char fixnv_property[50];
            char fixnv[95];
            int fixnv_size=0;

            fixnv[0] = '\0';
            strcpy(fixnv_property, RO_MODEM_FIXNVSIZE);

            property_get(fixnv_property, fixnv, "");
            if (0 == strlen(fixnv)) {
                ENG_LOG("invalid ro.modem.w.fixnv_size\n");
                return 0;
            }
            ENG_LOG("fixnv_property is %s fixnv %s\n", fixnv_property, fixnv);
            fixnv_size = strtol(fixnv, 0, 16);
            ENG_LOG("fixnv_size %x\n", fixnv_size);
            part_size = fixnv_size;
        }

        lpDiagAck->size = part_size; // prodnv size = 5M
        ENG_LOG("lpDiagAck->size=%d", lpDiagAck->size);

        close(fd);
out:
	free(fullpath);
	free(real_partname);

    }else{
            lpDiagAck->status = 1; //fail
    }

    *((char*)lpDiagAck +sizeof(diag_query_partition_info_ack_t)) = 0x7E;
    return msg_head_ptr->len+2;
}

int partinfo_read_offset(char *path, char *file, int offset, int size, unsigned char *buff)
{
    int szbuff=0;
    int namelen = 0;
    int ret = 0;

    ENG_LOG("%s enter",__FUNCTION__);
    memset(buff, 0, size);
    namelen = strlen(path)+strlen(file);
    char *fullpath = (char*)malloc(namelen + 1);
    if(!fullpath) {
        ENG_LOG("the %s malloc fail\n", __FUNCTION__);
        return 0;
    }
    memset(fullpath, 0, namelen);
    sprintf(fullpath, "%s%s", path, file);
    int fd = open(fullpath, O_RDONLY);
    if(fd < 0) {
	ENG_LOG("the %s--- read %s path fail\n", __FUNCTION__, fullpath);
        goto out;
    }

   ret = lseek(fd, offset,SEEK_SET);
   if(ret < 0) {
	   ENG_LOG("%s lseek fail.\n", __func__);
	   goto out;
   }
   szbuff = read(fd,buff,size);

out:
   free(fullpath);
   close(fd);

   return szbuff;
}

int partinfo_read(char *buf, int len, char *rsp, int rsplen){

    MSG_HEAD_T *msg_head_ptr = NULL;
    diag_read_partition_req_t *lpDiagRD = NULL;
    diag_read_partition_ack_t *lpDiagAck = NULL;
    char *real_partname = NULL;

    ENG_LOG("%s enter",__FUNCTION__);
    memcpy(rsp, buf, sizeof(MSG_HEAD_T)+1);
    msg_head_ptr = (MSG_HEAD_T*)(rsp+1);
    lpDiagAck = (diag_read_partition_ack_t*)(rsp+sizeof(MSG_HEAD_T)+1);
    lpDiagAck->cmd = 1;
    msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(diag_read_partition_ack_t);
    lpDiagRD = (diag_read_partition_req_t* )(buf + 1 + sizeof(MSG_HEAD_T));
    memset(lpDiagAck->data, 0, lpDiagRD->size);

    if(findpart(PARTINFO_PATH, lpDiagRD->partition_name, &real_partname)) {
        int offset = lpDiagRD->offset;
        int size = lpDiagRD->size;
        //read prodnv data: offset, size
        int retRD = partinfo_read_offset(PARTINFO_PATH, real_partname,
                                                 offset, size, lpDiagAck->data);
        if(real_partname != NULL)
            free(real_partname);
        msg_head_ptr->len += retRD;
        lpDiagAck->status = 0; //sucess
    }else{
        lpDiagAck->status = 1; //fail
    }

    *((char*)lpDiagAck +sizeof(diag_read_partition_ack_t)+(lpDiagRD->size)) = 0x7E;

    return msg_head_ptr->len+2;
}
int partinfo_write_offset(char *path, char *file, int offset, int size, unsigned char *buff, int szbuff){

    int namelen = 0;
    int ret = 0;

    namelen = strlen(path)+strlen(file);

    char *fullpath = (char*)malloc(namelen + 1);
    if(!fullpath) {
        ENG_LOG("the %s malloc fail\n", __FUNCTION__);
        return 0;
    }
    memset(fullpath, 0, namelen);
    sprintf(fullpath, "%s%s", path, file);

    int fd = open(fullpath, O_RDWR);
    if(fd < 0) {
	ENG_LOG("the %s--- open %s path fail\n", __FUNCTION__, fullpath);
        goto out;
    }

    ret = lseek(fd, offset,SEEK_SET);
    if(ret < 0) {
	ENG_LOG("%s lseek fail.\n", __func__);
	goto out;
    }
    szbuff=write(fd, buff,size);
    fsync(fd);
out:
     free(fullpath);
     close(fd);

     return szbuff;
}

int partinfo_write(char *buf, int len, char *rsp, int rsplen){

    MSG_HEAD_T* msg_head_ptr = NULL;
    diag_write_partition_req_t* lpDiagWR = NULL;
    diag_write_partition_ack_t* lpDiagAck = NULL;
    char *real_partname[4] = {NULL};
    int count=0;
    int i=0;

    ENG_LOG("%s enter",__FUNCTION__);
    memcpy(rsp, buf, sizeof(MSG_HEAD_T)+1);
    msg_head_ptr = (MSG_HEAD_T*)(rsp+1);
    lpDiagAck = (diag_write_partition_ack_t*)(rsp+sizeof(MSG_HEAD_T)+1);
    lpDiagAck->cmd = 2;
    msg_head_ptr->len = sizeof(MSG_HEAD_T)+sizeof(diag_write_partition_ack_t);

    lpDiagWR = (diag_write_partition_req_t* )(buf + 1 + sizeof(MSG_HEAD_T));
    count = get_match(PARTINFO_PATH, lpDiagWR->partition_name, &real_partname[0]);
    if(count == 0) {
        lpDiagAck->status = 1; //fail
    }
    while(i < count) {
        int offset = lpDiagWR->offset;
        int size = lpDiagWR->size;
        ENG_LOG("%s i=%d",__FUNCTION__, i);
        //write prodnv data: offset, size
        int retWR = partinfo_write_offset(PARTINFO_PATH, real_partname[i],
                                            offset, size, lpDiagWR->data, size);
        if(real_partname[i] != NULL)
            free(real_partname[i]);
        if (retWR == size){
            lpDiagAck->status = 0; //success
        }else{
            lpDiagAck->status = 1; //fail
            break;
        }
        i++;
    }


   *((char*)lpDiagAck +sizeof(diag_write_partition_ack_t)) = 0x7E;

   return msg_head_ptr->len+2;
}

int partinfo_handler(char *buf, int len, char *rsp, int rsplen){
      int ret = 0;

      diag_query_partition_info_req_t *lpDiagApReq = NULL;

      ENG_LOG("%s enter",__FUNCTION__);
      if(NULL == buf || NULL == rsp) {
          ENG_LOG("%s,null pointer",__FUNCTION__);
          return 0;
      }

      lpDiagApReq = (diag_query_partition_info_req_t* )(buf + 1 + sizeof(MSG_HEAD_T));
      switch(lpDiagApReq->cmd){
          case 0:
              ret = partinfo_query(buf,len,rsp,rsplen);
              break;
          case 1:
              ret = partinfo_read(buf,len,rsp,rsplen);
              break;
          case 2:
              ret = partinfo_write(buf,len,rsp,rsplen);
              break;
          default:
              break;
      }

    return ret;
}

void register_this_module_ext(struct eng_callback *reg, int *num)
{
    int moudles_num = 0;
    ENG_LOG("register_this_module_ext :libpartinfo");

    (reg+moudles_num)->type = 0x5D;
    (reg+moudles_num)->subtype = 0x08;
    (reg+moudles_num)->diag_ap_cmd = 0x00;
    (reg+moudles_num)->eng_diag_func = partinfo_handler;
     moudles_num++;

    (reg+moudles_num)->type = 0x5D;
    (reg+moudles_num)->subtype = 0x08;
    (reg+moudles_num)->diag_ap_cmd = 0x1;
    (reg+moudles_num)->eng_diag_func = partinfo_handler;
    moudles_num++;

    (reg+moudles_num)->type = 0x5D;
    (reg+moudles_num)->subtype = 0x08;
    (reg+moudles_num)->diag_ap_cmd = 0x2;
    (reg+moudles_num)->eng_diag_func = partinfo_handler;

    moudles_num++;
    *num = moudles_num;
}