#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"
#include "sci_if.h"

int dis_wq_post_cqe(struct dis_wq *diswq, 
                    struct dis_wqe *wqe,
                    enum ib_wc_status wq_status) 
{
    unsigned long flags;
    struct dis_cq* discq = diswq->discq;
    struct dis_cqe* cqe;
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&discq->cqe_lock, flags);

    cqe = discq->cqe_queue + (discq->cqe_put % discq->cqe_max);

    if(cqe->flags != DIS_WQE_FREE) {
        spin_unlock_irqrestore(&discq->cqe_lock, flags);
        return -42;
    }

    cqe->ibqp       = &diswq->disqp->ibqp;
    cqe->id         = wqe->id;
    cqe->opcode     = wqe->opcode;
    cqe->byte_len   = wqe->byte_len;
    cqe->status     = wq_status;
    cqe->flags      = DIS_WQE_VALID;

    discq->cqe_put++;

    spin_unlock_irqrestore(&discq->cqe_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

enum ib_wc_status dis_wq_consume_one_sqe(struct sci_if_msg *msg)
{
    int ret;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        ret = sci_if_receive_request(msg);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return IB_WC_SUCCESS;
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return IB_WC_RESP_TIMEOUT_ERR;
}

enum ib_wc_status dis_wq_consume_one_rqe(struct sci_if_msg *msg)
{
    int ret;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        ret = sci_if_send_request(msg);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return IB_WC_SUCCESS;
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return IB_WC_RESP_TIMEOUT_ERR;
}

int dis_wq_consume_all(struct dis_wq *diswq)
{
    int free;
    struct dis_wqe *wqe;
    struct sci_if_msg msg;
    enum ib_wc_status wc_status;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        wqe = diswq->wqe_queue + (diswq->wqe_get % diswq->wqe_max);

        if(wqe->flags != DIS_WQE_VALID) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }

        msg.msq     = &diswq->dismsq.msq;
        msg.msg     = (void *)(wqe->sg_list[0].addr); //TODO: Include all segments
        msg.size    = wqe->sg_list[0].length;
        msg.free    = &free;
        msg.flags   = 0; //TODO: SCIL_FLAG_SEND_RECEIVE_PAIRS_ONLY

        switch (diswq->wq_type) {
        case DIS_RQ:
            wc_status = dis_wq_consume_one_sqe(&msg);
            break;

        case DIS_SQ:
            wc_status = dis_wq_consume_one_rqe(&msg);
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        dis_wq_post_cqe(diswq, wqe, wc_status);
        wqe->flags = DIS_WQE_FREE;
        diswq->wqe_get++;
        return 0; //TODO: remove
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_wq_init(struct dis_wq *diswq)
{
    int ret, sleep_ms;
    pr_devel(DIS_STATUS_START);

    sleep_ms = DIS_QP_SLEEP_MS_INITIAL;
    while (!kthread_should_stop()) {
        switch (diswq->wq_type) {
        case DIS_RQ:
            ret = sci_if_create_msq(&diswq->dismsq);
            if(!ret) {
                pr_devel(DIS_STATUS_COMPLETE);
                return 0;
            }
            break;

        case DIS_SQ:
            ret = sci_if_connect_msq(&diswq->dismsq);
            if(!ret) {
                pr_devel(DIS_STATUS_COMPLETE);
                return 0;
            }
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        sleep_ms = min(sleep_ms + 10, DIS_QP_SLEEP_MS_MAX);
        msleep_interruptible(sleep_ms);
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

void dis_wq_exit(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    switch (diswq->wq_type) {
    case DIS_RQ:
        sci_if_remove_msq(&diswq->dismsq);
        break;

    case DIS_SQ:
        sci_if_disconnect_msq(&diswq->dismsq);
        break;

    default:
        pr_devel(DIS_STATUS_FAIL);
        return;
    }

    pr_devel(DIS_STATUS_COMPLETE);
}

int dis_wq_thread(void *diswq_buf)
{
    int ret, signal;
    struct dis_wq *diswq = (struct dis_wq*)diswq_buf;
    pr_devel(DIS_STATUS_START);
    
    diswq->wq_state = DIS_WQ_RUNNING;

    ret = dis_wq_init(diswq);
    if (ret) {
        diswq->wq_state = DIS_WQ_EXITED;
        pr_devel(DIS_STATUS_FAIL);
        return 0;
    }
 
    while (!kthread_should_stop()) {
        signal = wait_event_killable(diswq->wait_queue,
                                        diswq->wq_flag != DIS_WQ_EMPTY ||
                                        kthread_should_stop());
        if (signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        if (kthread_should_stop()) {
            pr_devel("Kernel thread should stop, exiting!");
            break;
        }

        ret = dis_wq_consume_all(diswq);
        if (ret) {
            break;
        }

        diswq->wq_flag = DIS_WQ_EMPTY;
    }

    dis_wq_exit(diswq);

    diswq->wq_state = DIS_WQ_EXITED;
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_init(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    
    init_waitqueue_head(&diswq->wait_queue);
    diswq->wq_state = DIS_WQ_INITIALIZED;
    diswq->wq_flag  = DIS_WQ_EMPTY;

    diswq->thread = kthread_create(dis_wq_thread, (void*)diswq, "DIS WQ Thread");//TODO: More descriptive name
    if (!diswq->thread) {
        pr_devel(DIS_STATUS_FAIL);
        diswq->wq_state = DIS_WQ_UNINITIALIZED;
        return -42;
    }
    
    wake_up_process(diswq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_notify(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    if (diswq->wq_state == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    diswq->wq_flag = DIS_WQ_POST;
    wake_up(&diswq->wait_queue);
    //TODO: Should we yield here?

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}


void dis_qp_exit(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    if (diswq->wq_state == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_COMPLETE);
        return;
    }

    kthread_stop(diswq->thread);
    wake_up(&diswq->wait_queue);
    wake_up_process(diswq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
}