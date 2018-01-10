#ifndef __ENG_SOCKET_H_
#define __ENG_SOCKET_H_

typedef struct{
char sn;
char log_type;
union{
    char location;
    char result;
};
}ENG_DATA_T;

void *eng_socket_thread(char *run_type);

#endif /*__ENG_SOCKET_H_*/