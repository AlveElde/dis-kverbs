#include "pr_fmt.h"

#include "sci_if_lib.h"

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
// EXPORT_SYMBOL(sci_if_create_msq);

void sci_if_remove_msq(struct sci_if_msq *msq)
{
    pr_devel(DIS_STATUS_START);
    SCILRemoveMsgQueue(&msq->msq, 0);
    pr_devel(DIS_STATUS_COMPLETE);
}
// EXPORT_SYMBOL(sci_if_remove_msq);

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
// EXPORT_SYMBOL(sci_if_connect_msq);

void sci_if_disconnect_msq(struct sci_if_msq *msq)
{
    pr_devel(DIS_STATUS_START);
    SCILDisconnectMsgQueue(&msq->msq, 0);
    pr_devel(DIS_STATUS_COMPLETE);
}
// EXPORT_SYMBOL(sci_if_disconnect_msq);

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
// EXPORT_SYMBOL(sci_if_handshake_msq);

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
// EXPORT_SYMBOL(sci_if_send_request);

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
// EXPORT_SYMBOL(sci_if_receive_request);