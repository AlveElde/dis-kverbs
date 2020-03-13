#include "pr_fmt.h"

#include "dis_msq.h"

int dis_create_msq(struct msq_ctx *msq, int retry_max)
{
    int i = 0;
    sci_error_t err;
    pr_devel(DIS_STATUS_START);

    for(i = 0; i < retry_max; i++) {
        err = SCILCreateMsgQueue(&(msq->msq), 
                                    msq->localAdapterNo, 
                                    msq->remoteNodeId, 
                                    msq->lmsqId,
                                    msq->rmsqId, 
                                    msq->maxMsgCount, 
                                    msq->maxMsgSize, 
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
EXPORT_SYMBOL(dis_create_msq);

int dis_connect_msq(struct msq_ctx *msq, int retry_max)
{
    int i = 0;
    sci_error_t err;
    pr_devel(DIS_STATUS_START);
    
    for(i = 0; i < retry_max; i++) {
        err = SCILConnectMsgQueue(&(msq->msq), 
                                    msq->localAdapterNo, 
                                    msq->remoteNodeId, 
                                    msq->lmsqId,
                                    msq->rmsqId, 
                                    msq->maxMsgCount, 
                                    msq->maxMsgSize, 
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
EXPORT_SYMBOL(dis_connect_msq);


int dis_send_request(struct msg_ctx *msg)
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
EXPORT_SYMBOL(dis_send_request);

int dis_receive_request(struct msg_ctx *msg, int retry_max)
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
EXPORT_SYMBOL(dis_receive_request);
