#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "dis_sci_if.h"

MODULE_DESCRIPTION("SCI Lib Interface");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static unsigned int timeout             = 0; // Not used
static unsigned int local_adapter_no    = 0;
static unsigned int remote_node_id      = 0;
static bool is_initiator                = true;
static bool use_l_qpn                   = true;

module_param(local_adapter_no, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(remote_node_id, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(is_initiator, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(use_l_qpn, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(local_adapter_no, "");
MODULE_PARM_DESC(remote_node_id, "");
MODULE_PARM_DESC(is_initiator, "");
MODULE_PARM_DESC(use_l_qpn, "");

#define SCIL_INIT_FLAGS 0
#define SCIL_EXIT_FLAGS 0

int dis_sci_if_create_msq(struct dis_wq *wq)
{
    int l_msq_id, r_msq_id;
    sci_error_t err;
#ifdef DIS_USE_DMA
    dis_nodeId_list_t remote_node_ids;
    sci_msq_attr_t attr;
    // pr_devel(DIS_STATUS_START);
#endif
    l_msq_id = wq->l_qpn;
    r_msq_id = use_l_qpn ?  wq->l_qpn :  wq->r_qpn;

    l_msq_id = is_initiator ? l_msq_id * 2 : (l_msq_id * 2) + 1;
    r_msq_id = is_initiator ? r_msq_id * 2 : (r_msq_id * 2) + 1;

    // pr_devel("Targeting remote_node_id %d", remote_node_id);
    // pr_devel("Creating MSQ with l_msq_id: %d, r_msq_id: %d, wqe_max: %d, mtu: %d", 
                // l_msq_id, r_msq_id, wq->wqe_max, wq->mtu);

#ifdef DIS_USE_DMA
    remote_node_ids[0] = remote_node_id;
    remote_node_ids[1] = 0;
    attr.pio_buf_size = 3*128*1024;
    attr.dma_buf_size = 3*128*1024;
    err = SCILCreateMsgQueueCb(&(wq->sci_msq), 
                                local_adapter_no, 
                                remote_node_ids,
                                l_msq_id,
                                r_msq_id,
                                &attr,
                                NULL,
                                NULL,
                                timeout,
                                SCIL_FLAG_USE_DMA);
#else
    err = SCILCreateMsgQueue(&(wq->sci_msq),
                                local_adapter_no,
                                remote_node_id,
                                l_msq_id,
                                r_msq_id,
                                DIS_MSG_MAX,
                                DIS_MSG_SIZE_MAX,
                                timeout,
                                0);
#endif
    switch (err)
    {
    case SCI_ERR_OK:
        pr_devel(DIS_STATUS_COMPLETE);
        return 0;
    case SCI_ERR_ILLEGAL_PARAMETER:
        // pr_devel("SCI_ERR_ILLEGAL_PARAMETER");
        return -42;
    case SCI_ERR_NOSPC:
        // pr_devel("SCI_ERR_NOSPC");
        return -42;
    default:
        // pr_devel("Unknown error: %d" DIS_STATUS_FAIL, err);
        return -42;
    }
}
EXPORT_SYMBOL(dis_sci_if_create_msq);

void dis_sci_if_remove_msq(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);
    SCILRemoveMsgQueue(&wq->sci_msq, 0);
    pr_devel(DIS_STATUS_COMPLETE);
}
EXPORT_SYMBOL(dis_sci_if_remove_msq);

int dis_sci_if_connect_msq(struct dis_wq *wq)
{
    int l_msq_id, r_msq_id;
    sci_error_t err;
#ifdef DIS_USE_DMA
    dis_nodeId_list_t remote_node_ids;
    sci_msq_attr_t attr;
#endif
    // pr_devel(DIS_STATUS_START);

    l_msq_id = wq->l_qpn;
    r_msq_id = use_l_qpn ?  wq->l_qpn :  wq->r_qpn;

    l_msq_id = is_initiator ? (l_msq_id * 2) + 1 : l_msq_id * 2;
    r_msq_id = is_initiator ? (r_msq_id * 2) + 1 : r_msq_id * 2;

    // pr_devel("Targeting remote_node_id %d", remote_node_id);
    // pr_devel("Connecting MSQ with l_msq_id: %d, r_msq_id: %d, wqe_max: %d, mtu: %d", 
                // l_msq_id, r_msq_id, wq->wqe_max, wq->mtu);

#ifdef DIS_USE_DMA
    remote_node_ids[0] = remote_node_id;
    remote_node_ids[1] = 0;
    attr.pio_buf_size = 3*128*1024;
    attr.dma_buf_size = 3*128*1024;
    err = SCILConnectMsgQueueCb(&(wq->sci_msq), 
                                    local_adapter_no, 
                                    remote_node_ids,
                                    l_msq_id,
                                    r_msq_id,
                                    &attr,
                                    NULL,
                                    NULL,
                                    timeout,
                                    SCIL_FLAG_USE_DMA);            

#else
    err = SCILConnectMsgQueue(&(wq->sci_msq), 
                                local_adapter_no, 
                                remote_node_id, 
                                l_msq_id,
                                r_msq_id,
                                DIS_MSG_MAX,
                                DIS_MSG_SIZE_MAX,
                                timeout, 
                                0);
#endif
    switch (err)
    {
    case SCI_ERR_OK:
        pr_devel(DIS_STATUS_COMPLETE);
        return 0;
    case SCI_ERR_CONNECTION_REFUSED:
        // pr_devel("SCI_ERR_CONNECTION_REFUSED");
        return -42;
    case SCI_ERR_NO_SUCH_SEGMENT:
        // pr_devel("SCI_ERR_NO_SUCH_SEGMENT");
        return -42;
    default:
        // pr_devel("Unknown error: %d", err);
        return -42;
    }
}
EXPORT_SYMBOL(dis_sci_if_connect_msq);

void dis_sci_if_disconnect_msq(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);
    SCILDisconnectMsgQueue(&wq->sci_msq, 0);
    pr_devel(DIS_STATUS_COMPLETE);
}
EXPORT_SYMBOL(dis_sci_if_disconnect_msq);

int dis_sci_if_send_v_msg(struct dis_wqe *wqe)
{
    int free;
    sci_error_t err;
    // pr_devel(DIS_STATUS_START);

#ifdef DIS_USE_DMA
    err = SCILSendVMsgDma(*(wqe->sci_msq),
                            &wqe->sci_msg,
                            wqe->byte_len,
                            &free,
                            4,
                            NULL,
                            SCIL_FLAG_MSG_FLUSH | 
                            SCIL_FLAG_SEND_RECEIVE_PAIRS_ONLY |
                            SCIL_FLAG_USE_DMA
                            );
#else
    err = SCILSendVMsg(*(wqe->sci_msq),
                        &wqe->sci_msg,
                        wqe->byte_len,
                        &free,
                        SCIL_FLAG_MSG_FLUSH |
                        // SCIL_FLAG_FULL_ERROR_CHECK |
                        SCIL_FLAG_SEND_RECEIVE_PAIRS_ONLY
                        );
#endif
    switch (err)
    {
    case SCI_ERR_OK:
        // pr_devel(DIS_STATUS_COMPLETE);
        return 0;
    case SCI_ERR_EWOULD_BLOCK:
        // pr_devel("SCI_ERR_EWOULD_BLOCK: " DIS_STATUS_FAIL);
        return -42;
    case SCI_ERR_NOT_CONNECTED:
        // pr_devel("SCI_ERR_NOT_CONNECTED: " DIS_STATUS_FAIL);
        return -42;
    case SCI_ERR_OUT_OF_RANGE:
        // pr_devel("SCI_ERR_OUT_OF_RANGE: " DIS_STATUS_FAIL);
        return -42;
    default:
        // pr_devel("Unknown error code: " DIS_STATUS_FAIL);
        return -42;
    }
}
EXPORT_SYMBOL(dis_sci_if_send_v_msg);

int dis_sci_if_receive_v_msg(struct dis_wqe *wqe)
{
    int free;
    sci_error_t err;
    // pr_devel(DIS_STATUS_START);

    err = SCILReceiveVMsg(*(wqe->sci_msq),
                        &wqe->sci_msg,
                        &wqe->byte_len,
                        &free,
                        // SCIL_FLAG_FULL_ERROR_CHECK |
                        SCIL_FLAG_SEND_RECEIVE_PAIRS_ONLY
                        // SCIL_FLAG_DGRAM
                        );
    switch (err)
    {
    case SCI_ERR_OK:
        // pr_devel(DIS_STATUS_COMPLETE);
        return 0;
    case SCI_ERR_EWOULD_BLOCK:
        // pr_devel("SCI_ERR_EWOULD_BLOCK");
        return -42;
    case SCI_ERR_NOT_CONNECTED:
        // pr_devel("SCI_ERR_NOT_CONNECTED: " DIS_STATUS_FAIL);
        return -42;
    case SCI_ERR_ILLEGAL_PARAMETER:
        pr_devel("SCI_ERR_ILLEGAL_PARAMETER: " DIS_STATUS_FAIL);
        return -42;
    case SCI_ERR_OUT_OF_RANGE:
        // pr_devel("SCI_ERR_OUT_OF_RANGE: " DIS_STATUS_FAIL);
        return -42;
    default:
        // pr_devel("Unknown error code: " DIS_STATUS_FAIL);
        return -42;
    }
}
EXPORT_SYMBOL(dis_sci_if_receive_v_msg);

static int __init dis_sci_if_init(void)
{
    sci_error_t ret;
    pr_devel(DIS_STATUS_START);
    
    ret = SCILInit(SCIL_INIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return -1;
    }
    
    ret = SCILReconf(SCIL_INIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return -1;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit dis_sci_if_exit(void)
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

module_init(dis_sci_if_init);
module_exit(dis_sci_if_exit);