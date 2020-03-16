#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>

// #include "sci_if.h"
#include "dis_verbs.h"

MODULE_DESCRIPTION("SCI Lib Interface");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static unsigned int local_adapter_no    = 99;
static unsigned int remote_node_id      = 99;
static bool is_initiator                = true;

module_param(local_adapter_no, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(remote_node_id, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(is_initiator, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(local_adapter_no, "");
MODULE_PARM_DESC(remote_node_id, "");
MODULE_PARM_DESC(is_initiator, "");

#define SCIL_INIT_FLAGS 0
#define SCIL_EXIT_FLAGS 0

int sci_if_create_msq(struct sci_if_msq *msq, int retry_max)
{
    int i = 0;
    sci_error_t err;
    pr_devel(DIS_STATUS_START);

    for(i = 0; i < retry_max; i++) {
        err = SCILCreateMsgQueue(&(msq->msq),
                                    local_adapter_no,
                                    remote_node_id, 
                                    msq->lmsq_id,
                                    msq->rmsq_id,
                                    msq->max_msg_count,
                                    msq->max_msg_size,
                                    msq->timeout,
                                    msq->flags);
        switch (err)
        {
        case SCI_ERR_OK:
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        case SCI_ERR_ILLEGAL_PARAMETER:
            pr_devel("SCI_ERR_ILLEGAL_PARAMETER: " DIS_STATUS_FAIL);
            return -42;
        case SCI_ERR_NOSPC:
            pr_devel("SCI_ERR_NOSPC: " DIS_STATUS_FAIL);
            return -42;
        default:
            pr_devel("Sleeping and retrying.. %d", err);
        }

        if(i + 1 < retry_max) {
            msleep(1000);
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

void sci_if_remove_msq(struct sci_if_msq *msq)
{
    pr_devel(DIS_STATUS_START);
    SCILRemoveMsgQueue(&msq->msq, 0);
    pr_devel(DIS_STATUS_COMPLETE);
}

int sci_if_connect_msq(struct sci_if_msq *msq, int retry_max)
{
    int i = 0;
    sci_error_t err;
    pr_devel(DIS_STATUS_START);
    
    for(i = 0; i < retry_max; i++) {
        err = SCILConnectMsgQueue(&(msq->msq), 
                                    local_adapter_no, 
                                    remote_node_id, 
                                    msq->lmsq_id,
                                    msq->rmsq_id, 
                                    msq->max_msg_count, 
                                    msq->max_msg_size, 
                                    msq->timeout, 
                                    msq->flags);
        switch (err)
        {
        case SCI_ERR_OK:
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        case SCI_ERR_CONNECTION_REFUSED:
            pr_devel("SCI_ERR_CONNECTION_REFUSED: " DIS_STATUS_FAIL);
            return -42;
        case SCI_ERR_NO_SUCH_SEGMENT:
            pr_devel("SCI_ERR_NOSPC: " DIS_STATUS_FAIL);
            return -42;
        default:
            pr_devel("Sleeping and retrying.. %d", err);
        }

        if(i + 1 < retry_max) {
            msleep(1000);
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

void sci_if_disconnect_msq(struct sci_if_msq *msq)
{
    pr_devel(DIS_STATUS_START);
    SCILDisconnectMsgQueue(&msq->msq, 0);
    pr_devel(DIS_STATUS_COMPLETE);
}

int sci_if_handshake_msq(struct sci_if_msq *msq, int retry_max) 
{
    int ret;

    if(is_initiator) {
        ret = sci_if_create_msq(msq, retry_max);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        ret = sci_if_connect_msq(msq, retry_max);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            sci_if_remove_msq(msq);
            return -42;
        }
    } else {
        ret = sci_if_connect_msq(msq, retry_max);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        ret = sci_if_create_msq(msq, retry_max);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            sci_if_disconnect_msq(msq);
            return -42;
        }
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int sci_if_send_request(struct sci_if_msg *msg)
{
    sci_error_t err;
    pr_devel(DIS_STATUS_START);

    err = SCILSendMsg(*(msg->msq),
                        msg->msg,
                        msg->size,
                        msg->free,
                        msg->flags);
    switch (err)
    {
    case SCI_ERR_OK:
        pr_devel(DIS_STATUS_COMPLETE);
        return 0;
    case SCI_ERR_EWOULD_BLOCK:
        pr_devel("SCI_ERR_EWOULD_BLOCK: " DIS_STATUS_FAIL);
        return -42;
    case SCI_ERR_NOT_CONNECTED:
        pr_devel("SCI_ERR_NOT_CONNECTED: " DIS_STATUS_FAIL);
        return -42;
    default:
        pr_devel("Unknown error code: " DIS_STATUS_FAIL);
        return -42;
    }
}

int sci_if_receive_request(struct sci_if_msg *msg, int retry_max)
{
    int i;
    sci_error_t err;
    pr_devel(DIS_STATUS_START);

    for(i = 0; i < retry_max; i++) {
        err = SCILReceiveMsg(*(msg->msq),
                            msg->msg,
                            msg->size,
                            msg->free,
                            msg->flags);
        switch (err)
        {
        case SCI_ERR_OK:
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        case SCI_ERR_EWOULD_BLOCK:
            pr_devel("SCI_ERR_EWOULD_BLOCK");
            break;
        case SCI_ERR_NOT_CONNECTED:
            pr_devel("SCI_ERR_NOT_CONNECTED: " DIS_STATUS_FAIL);
            return -42;
        case SCI_ERR_ILLEGAL_PARAMETER:
            pr_devel("SCI_ERR_ILLEGAL_PARAMETER: " DIS_STATUS_FAIL);
            return -42;
        default:
            pr_devel("Unknown error code: " DIS_STATUS_FAIL);
            return -42;
        }

        if(i + 1 < retry_max) {
            msleep(1000);
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

static int __init sci_if_init(void)
{
    sci_error_t ret;
    pr_devel(DIS_STATUS_START);
    
    ret = SCILInit(SCIL_INIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return -1;
    }

    

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit sci_if_exit(void)
{
    sci_error_t ret;
    pr_devel(DIS_STATUS_START);
    
    ret = SCILDestroy(SCIL_EXIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return;
    }

    pr_devel(DIS_STATUS_COMPLETE);
}

module_init(sci_if_init);
module_exit(sci_if_exit);