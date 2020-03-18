#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"
#include "sci_if.h"


int dis_wq_consume_rqe(struct sci_if_msg *msg)
{
    int ret, sleep_ms;
    pr_devel(DIS_STATUS_START);

    sleep_ms = DIS_QP_SLEEP_MS_INITIAL;
    while (!kthread_should_stop()) {
        //TODO: Introduce error codes to exit on lost connection
        ret = sci_if_receive_request(msg);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }

        sleep_ms = min(sleep_ms + 10, DIS_QP_SLEEP_MS_MAX);
        msleep_interruptible(sleep_ms);
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_wq_consume_sqe(struct sci_if_msg *msg)
{
    int ret, sleep_ms;
    pr_devel(DIS_STATUS_START);

    sleep_ms = DIS_QP_SLEEP_MS_INITIAL;
    while (!kthread_should_stop()) {
        //TODO: Introduce error codes to exit on lost connection
        ret = sci_if_send_request(msg);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }

        sleep_ms = min(sleep_ms + 10, DIS_QP_SLEEP_MS_MAX);
        msleep_interruptible(sleep_ms);
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_wq_init(struct dis_wq *diswq)
{
    int ret, sleep_ms;
    pr_devel(DIS_STATUS_START);

    diswq->dismsq.l_qpn         = diswq->l_qpn;
    diswq->dismsq.r_qpn         = diswq->r_qpn;
    diswq->dismsq.max_msg_count = 16;
    diswq->dismsq.max_msg_size  = 128;
    diswq->dismsq.timeout       = 1234;
    diswq->dismsq.flags         = 0;

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

int dis_wq_consume_all(struct dis_wq *diswq)
{
    int ret, free;
    struct dis_wqe *wqe;
    struct sci_if_msg msg;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        wqe = diswq->wqe + (diswq->wqe_get % diswq->max_wqe);

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
            //TODO: Introduce error codes to exit on lost connection
            ret = dis_wq_consume_rqe(&msg);
            if (ret) {
                pr_devel(DIS_STATUS_FAIL);
                return -42;
            }
            pr_devel("Successfully received message from requester: %s", 
                            (char *)(wqe->sg_list[0].addr));
            break;

        case DIS_SQ:
            //TODO: Introduce error codes to exit on lost connection
            ret = dis_wq_consume_sqe(&msg);
            if (ret) {
                pr_devel(DIS_STATUS_FAIL);
                return -42;
            }
            pr_devel("Successfully sent message to requester: %s", 
                            (char *)(wqe->sg_list[0].addr));
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }
        
        wqe->flags = DIS_WQE_FREE;
        diswq->wqe_get++;
        return 0; //TODO: remove
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
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

int dis_qp_post(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    if (diswq->wq_state == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    diswq->wq_flag = DIS_WQ_POST;
    wake_up(&diswq->wait_queue);

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