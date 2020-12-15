#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "stdio.h"
#include <errno.h>
#include <utils/Log.h>
#include <time.h>
#include "cutils/properties.h"
#include <pthread.h>
#include "at_tok.h"
#include "sprd_fts_type.h"

#define LOG_TAG                       "FACTORYTESTRADIO"
#define TEL_TIMEOUT                   100
#define AT_BUFFER_SIZE                512
#define MAX_PHONE_MUM                 2
#define MAX_AT_CHANNEL_NAME_LENGHT    32

static const char *s_at_call = "AT+TELTEST";
static const char *s_at_sim = "AT+SIMTEST";

static char s_ATBuffer[AT_BUFFER_SIZE];
static char *s_ATBufferCur = s_ATBuffer;
static bool s_threadStarted = false;

DYMIC_WRITETOPC_FUNC g_func[WRITE_TO_MAX] = {NULL};

char g_lte_normal_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT] = {
    "/dev/stty_lte1",
    "/dev/stty_lte4",
};

char g_nr_normal_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT] = {
    "/dev/stty_nr1",
    "/dev/stty_nr4",
};

/* static report_ptr g_func; */
int send_at_to_engpc(DYMIC_WRITETOPC_FUNC *write_interface_ptr) {
    ALOGD("send_at_to_engpc");
    for (int i = 0; i < WRITE_TO_MAX; i++) {
        g_func[i] = write_interface_ptr[i];
        if (g_func[i] != NULL) {
            ALOGD("send_at_to_engpc ad 0x%x, i:%d", g_func[i], i);
        }
    }

    return 0;
}

bool isLteOnly(int phoneId) {
    char testmode[PROPERTY_VALUE_MAX + 1] = {0};
    property_get("persist.vendor.radio.modem.workmode", testmode, "0,0");
    ALOGD("isLteOnly: persist.vendor.radio.modem.workmode = %s\n", testmode);

    if (phoneId == 0) {
        return (testmode[0] == '3');
    } else {
        return (testmode[2] == '3');
    }
}

bool isNrMode() {
    char modemtype[PROPERTY_VALUE_MAX + 1] = {0};
    property_get("ro.vendor.radio.modemtype", modemtype, "");
    ALOGD("isNrMode: ro.vendor.radio.modemtype = %s\n", modemtype);

    if (strcmp(modemtype, "nr")) {
        return false;
    }

    return true;
}

static char *findNextEOL(char *cur) {
    if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0') {
        /* SMS prompt character...not \r terminated */
        return cur+2;
    }

    // Find next newline
    while (*cur != '\0' && *cur != '\r' && *cur != '\n') cur++;

    return *cur == '\0' ? NULL : cur;
}

static char *readline(int modemfd) {
    ssize_t count;
    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret = NULL;

    /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */

    if (*s_ATBufferCur == '\0') {
        /* empty buffer */
        s_ATBufferCur = s_ATBuffer;
        *s_ATBufferCur = '\0';
        p_read = s_ATBuffer;
    } else {   /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */

        // skip over leading newlines
        while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n') {
            s_ATBufferCur++;
        }

        p_eol = findNextEOL(s_ATBufferCur);

        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(s_ATBufferCur);
            memmove(s_ATBuffer, s_ATBufferCur, len + 1);
            p_read = s_ATBuffer + len;
            s_ATBufferCur = s_ATBuffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }

    while (p_eol == NULL) {
        if (0 == AT_BUFFER_SIZE - (p_read - s_ATBuffer)) {
            /* ditch buffer and start over again */
            s_ATBufferCur = s_ATBuffer;
            *s_ATBufferCur = '\0';
            p_read = s_ATBuffer;
        }

        do {
            count = read(modemfd, p_read, AT_BUFFER_SIZE - (p_read - s_ATBuffer));
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            p_read[count] = '\0';
            // skip over leading newlines
            while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n') {
                s_ATBufferCur++;
            }

            p_eol = findNextEOL(s_ATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */
    ret = s_ATBufferCur;
    *p_eol = '\0';
    s_ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
    /* and there will be a \0 at *p_read */
    return ret;
}

int sendATCmd(int fd, char* cmd, char* buf, unsigned int buf_len, int wait) {
    int ret = -1;
    int cmd_len = 0;
    char *line = NULL;
    fd_set readfs;
    struct timeval timeout;


    if (NULL == cmd) {
        ALOGE("error param");
        return -1;
    }

    ALOGD("[fd:%d] >>>> at_cmd: %s", fd, cmd);
    cmd_len = strlen(cmd);
    ret = write(fd, cmd, cmd_len);

    if (ret != cmd_len) {
        ALOGE("mmitest write err, ret = %d, cmd_len = %d\n", ret, cmd_len);
        return -1;
    }

    cmd_len = 0;

    write(fd, "\r", 1);

    if (wait <= 0) wait = 5;

    for (;;) {
        timeout.tv_sec = wait;
        timeout.tv_usec = 0;
        FD_ZERO(&readfs);
        FD_SET(fd, &readfs);
        ret = select(fd+1, &readfs, NULL, NULL, &timeout);

        if (ret < 0) {
            ALOGE("mmitest select err ");
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            } else {
                return -1;
            }
        } else if (ret == 0) {
            ALOGD("mmitest select time out");
            return -1;
        } else {
            /* go to recv response*/
            break;
        }
    }

    for (;;) {
        line = readline(fd);
        ALOGD("mmitest %s [fd:%d] <<<< at_rsp: %s", cmd, fd, line);

        if (NULL == line) {
            ALOGD("line is NULL");
            break;
        }

        if (strstr(line, "OK")) {
            ret = cmd_len;
            break;
        } else if ((buf_len == 0 || buf == NULL) && strstr(line, "ERROR")) {
            ret = -1;
            break;
        } else {
            if (buf_len == 0 || buf == NULL) {
                continue;
            }
            if (cmd_len + strlen(line) > buf_len) {
                ALOGD("mmitest recv too many word, %d > %d\n", cmd_len + strlen(line),
                        buf_len);
                ret = -1;
                break;
            }
            memcpy(buf + cmd_len, line, strlen(line));
            cmd_len += strlen(line);

            // get +CME ERROR: xxx
            if (strstr(buf, "ERROR")) {
                ret = -1;
                break;
            }
        }
    }

    return ret;
}

void *readURCThread() {
    int ret = -1;
    int fd0 = -1;
    int fd3 = -1;
    int fd = -1;
    int count = 0;
    fd_set readfs;
    fd_set rdfds;
    char buffer[AT_BUFFER_SIZE] = {0};

    if (isNrMode()) {
        fd0 = open("/dev/stty_nr0", O_RDWR | O_NONBLOCK);
        fd3 = open("/dev/stty_nr3", O_RDWR | O_NONBLOCK);
    } else {
        fd0 = open("/dev/stty_lte0", O_RDWR | O_NONBLOCK);
        fd3 = open("/dev/stty_lte3", O_RDWR | O_NONBLOCK);
    }
    ALOGD("read urc fd0 = %d, fd3 = %d", fd0, fd3);

    FD_ZERO(&readfs);
    FD_SET(fd0, &readfs);
    FD_SET(fd3, &readfs);
    pthread_detach(pthread_self());

    for (;;) {
        do {
            rdfds = readfs;
            ret = select((fd0 > fd3) ? (fd0 + 1) : (fd3 + 1), &rdfds,
                    NULL, NULL, NULL);
        } while (ret == -1 && errno == EINTR);

        if (ret <= 0) {
            ALOGD("modem error !");
            s_threadStarted = false;
            return NULL;
        }

        if (FD_ISSET(fd0, &rdfds)) {
            fd = fd0;
        } else if (FD_ISSET(fd3, &rdfds)) {
            fd = fd3;
        }

        ALOGD("read urc fd = %d", fd);
        do {
            count = read(fd, buffer, AT_BUFFER_SIZE);
        } while (count < 0 && errno == EINTR);

        ALOGD("URC: %d, line = %s", fd, buffer);
    }

    return NULL;
}

//AT+TELTEST=p1,p2,str   p1 is dial(1)/hold(0); p2 is sim1/2; str is number
static int TestMakeCall(char *req, char *rsp) {
    ALOGD("test CALL start: req = %s\n", req);
    int ret = 0;
    int operate = -1;
    int simNum = -1;
    int mode = -1;
    int fdCall = -1;
    char *number = NULL;
    char *ptr = NULL;
    char *path = NULL;
    char *ptmp = NULL;
    char cmd[32] = {0};
    char tmp[AT_BUFFER_SIZE] = {0};
    time_t startTime, nowTime;
    pthread_t tid;
    pthread_attr_t attr;

    ptr = strdup(req) ;
    char *temp = ptr;
    at_tok_flag_start(&temp, '=');
    at_tok_nextint(&temp, &operate);
    at_tok_nextint(&temp, &simNum);
    at_tok_nextstr(&temp, &number);

    ALOGD("operate = %d, SIM num = %d, CALL number = %s\n", operate, simNum, number);

    if ((simNum < 1) || (simNum > 2) || (operate < 0) || (operate > 1)) {
        ALOGE("SIM num = %d error or operate = %d error!\n", simNum, operate);
        free(ptr);
        return -1;
    }

    if (isNrMode()) {
        path = g_nr_normal_at_channel[simNum - 1];
    } else {
        path = g_lte_normal_at_channel[simNum - 1];
    }

    ALOGD("path = %s\n", path);
    fdCall = open(path, O_RDWR | O_NONBLOCK);

    if (fdCall < 0) {
        ALOGE("open failed %s\n", path);
        free(ptr);
        return -1;
    }

    //stop call
    if (0 == operate) {
#if defined(AUDIO_WHALE_HAL)
        if (g_func[WRITE_TO_NPISO_AT] != NULL) {
            char req_open[] = "AT+AUDIOPIPE=set_mode=0";
            ALOGD("enter: send at %s to audio", req_open);
            g_func[WRITE_TO_NPISO_AT](req_open, sizeof(req_open));
        }
#endif
        sendATCmd(fdCall, "ATH", NULL, 0, 0); //hang up
        sendATCmd(fdCall, "AT", NULL, 0, 0);
        close(fdCall);
        free(ptr);
        return 0;
    }

    if (!strcmp(number, "112")) {
        strcpy(cmd, "ATD112@1,#;");
    } else if (!strcmp(number, "10086")) {
        strcpy(cmd, "ATD10086;");
    } else {
        ALOGE("number = %s error!\n", number);
        close(fdCall);
        free(ptr);
        return -1;
    }

    free(ptr);
    sendATCmd(fdCall, "AT+SFUN=5", NULL, 0, 0); //close protocol
    usleep(3000 * 1000);

    if (isLteOnly(simNum)) {
        sendATCmd(fdCall, "AT+SPTESTMODEM=21,254", NULL, 0, 0);
    } else {
        sendATCmd(fdCall, "AT+SPTESTMODEM=15,10", NULL, 0, 0);
    }

    /*
     * open protocol stack and wait 100s
     * if exceed more than 20s, we regard rregistering network fail
    */
    sendATCmd(fdCall, "AT+SFUN=4", NULL, 0, 100);
    startTime = time(NULL);

    if (!s_threadStarted) {
        ALOGD("start to read urc");
        pthread_attr_init(&attr);
        s_threadStarted = true;
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&tid, &attr, readURCThread, NULL) < 0) {
            ALOGE("Failed to create read urc thread!");
            s_threadStarted = false;
        }
    }

    for (;;) {
        if (sendATCmd(fdCall, "AT+CREG?", tmp, sizeof(tmp), 0) >= 0) {
            ptmp = strstr(tmp, "CREG");
            if (ptmp) {
                at_tok_start(&ptmp);
                at_tok_nextint(&ptmp, &mode);
                ALOGD("get ps mode = %d", mode);

                if (2 == mode) {
                    at_tok_nextint(&ptmp, &mode);
                    ALOGD("get state = %d", mode);
                    if ((1 == mode) || (8 == mode) || (5 == mode)) {
                        break;
                    }
                }
            }
        }

        sleep(2);
        nowTime = time(NULL);
        if (nowTime - startTime > TEL_TIMEOUT ) {
            ALOGE("mmitest tel test failed");
            close(fdCall);
            return -1;
        }
    }

    ret = sendATCmd(fdCall, cmd, NULL,0, 0); //call 112
    ALOGD("tel send at return: %d", ret);
    usleep(200 * 1000);

    //open speaker
#if defined(AUDIO_WHALE_HAL)
    if (g_func[WRITE_TO_NPISO_AT] != NULL) {
        char req_open[] = "AT+AUDIOPIPE=test_out_stream_route=2;set_mode=2";
        ALOGD("enter: send at %s to audio", req_open);
        g_func[WRITE_TO_NPISO_AT](req_open, sizeof(req_open));
    }
#else
    sendATCmd(fdCall, "AT+SSAM=1", NULL, 0, 0);
#endif

    sleep(5);
    close(fdCall);
    return ret;
}

//AT+SIMTEST=p
static int TestCheckSIM (char *req, char *rsp) {
    ALOGD("test SIM start: req = %s\n", req);
    int ret = 0;
    int simNum = -1;
    int fd = -1;
    int simState = -1;
    int cnt = 10;
    char *path = NULL;
    char *ptmp = NULL;
    char *ptr = NULL;
    const char *cmd = "AT+EUICC?";
    char tmp[AT_BUFFER_SIZE] = {0};

    ptr = strdup(req);
    char *temp = ptr;
    at_tok_flag_start(&temp, '=');
    at_tok_nextint(&temp, &simNum);
    free(ptr);

    ALOGD("SIM num = %d\n", simNum);
    if ((simNum < 1) || (simNum > 2)) {
        ALOGE("SIM num = %d error!\n", simNum);
        return -1;
    }

    if (isNrMode()) {
        path = g_nr_normal_at_channel[simNum - 1];
    } else {
        path = g_lte_normal_at_channel[simNum - 1];
    }

    fd = open(path, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        ALOGE("open failed %s\n", path);
        return -1;
    }

    do {
        ret = sendATCmd(fd, "AT+CPIN?", tmp, sizeof(tmp), 0);
        if (ret >= 0 || (ret == -1 && !strstr(tmp, "+CME ERROR: 14"))) {
            break;
        }
        ALOGD("sim busy, retry");
        usleep(200 * 1000);
        cnt--;
    } while(cnt > 0);

    ALOGD("sendATCmd: simNum = %d, cmd = %s, path = %s", simNum, cmd, path);
    memset(tmp, 0, sizeof(tmp));
    if (sendATCmd(fd, cmd, tmp, sizeof(tmp), 0) < 0) {
        simState = -1;
    } else {
        ptmp = tmp;
        ptmp = strstr(ptmp, "EUICC");
        if (ptmp) {
            at_tok_start(&ptmp);
            at_tok_nextint(&ptmp, &simState);
        } else {
            simState = -1;
        }
    }
    ALOGD("get sim%d, state = %d", simNum, simState);
    close(fd);
    return (simState == 0) ? 0 : -1;
}

void register_this_module_ext(struct eng_callback *reg, int *num) {
    int moudles_num = 0;
    ALOGI("register_this_module_ext");

    sprintf((reg + moudles_num)->at_cmd, "%s", s_at_call);
    (reg + moudles_num)->eng_linuxcmd_func = TestMakeCall;
    (reg + moudles_num)->eng_set_writeinterface_func = send_at_to_engpc;
    (reg + moudles_num)->subtype = 0x68;
    ALOGV("register_this_module_ext TELE_CALL_AT:%p", TestMakeCall);
    moudles_num++;

    sprintf((reg + moudles_num)->at_cmd, "%s", s_at_sim);
    (reg + moudles_num)->eng_linuxcmd_func = TestCheckSIM;
    (reg + moudles_num)->subtype = 0x68;
    moudles_num++;
    ALOGV("register_this_module_ext TELE_SIM_AT:%p", TestCheckSIM);

    *num = moudles_num;
    ALOGI("register_this_module_ext:%d", *num);
}
