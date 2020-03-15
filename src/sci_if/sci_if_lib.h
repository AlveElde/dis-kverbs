#ifndef __SCI_IF_LIB_H__
#define __SCI_IF_LIB_H__

#include "scilib.h"

#define SCI_IF_TIMEOUT_SEC 10
#define MSG "Hello There!\n"
#define MSG_LEN strlen(MSG) + 1

static unsigned int local_adapter_no    = 99;
static unsigned int remote_node_id      = 99;
static bool is_initiator                = true;

struct sci_if_msq_duplex {
    sci_msq_queue_t incoming_msq;
    sci_msq_queue_t outgoing_msq;
    unsigned int    local_adapter_no; 
    unsigned int    remote_node_id;   
    unsigned int    lmsq_id;
    unsigned int    rmsq_id;
    unsigned int    max_msg_count;    
    unsigned int    max_msg_size;  
    unsigned int    timeout;
    unsigned int    flags;
};

struct sci_if_msq {
    sci_msq_queue_t msq;
    unsigned int    local_adapter_no; 
    unsigned int    remote_node_id;   
    unsigned int    lmsq_id;
    unsigned int    rmsq_id;
    unsigned int    max_msg_count;    
    unsigned int    max_msg_size;  
    unsigned int    timeout;
    unsigned int    flags;
};

struct sci_if_msg {
    sci_msq_queue_t *msq;
    void            *msg;    
    unsigned int    size;
    unsigned int    *free;
    unsigned int    flags;
};

#endif /* __SCI_IF_LIB_H__ */