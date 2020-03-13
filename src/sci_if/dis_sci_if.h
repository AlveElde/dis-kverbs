#ifndef __DIS_SCILIB_IF_H__
#define __DIS_SCILIB_IF_H__

#include "scilib.h"

#define MSG "Hello There!\n"
#define MSG_LEN strlen(MSG) + 1

struct dis_msq {
    sci_msq_queue_t msq;
    unsigned int    localAdapterNo; 
    unsigned int    remoteNodeId;   
    unsigned int    lmsqId;
    unsigned int    rmsqId;
    unsigned int    maxMsgCount;    
    unsigned int    maxMsgSize;  
    unsigned int    timeout;
    unsigned int    flags;
};

struct dis_msq_msg {
    sci_msq_queue_t *msq;
    void            *msg;    
    unsigned int    size;
    unsigned int    *free;
    unsigned int    flags;
};

#endif /* __DIS_SCILIB_IF_H__ */