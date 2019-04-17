#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "CProtol.h"

CProtol::CProtol(){
}

CProtol::~CProtol(){
}

int CProtol::decode(char* buff, int nlen){
    //info("decode");
    return nlen;
}

int CProtol::encode(char* buff, int nlen){
    //info("encode");
    return nlen;
}

int CProtol::checkframe(char* buff, int nlen){
    //info("checkframe");
    return 0;
}
