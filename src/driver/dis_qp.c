#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"
#include "sci_if.h"

static 

int dis_rq_init(struct dis_wq *diswq)
{
    int ret, sleep_ms = DIS_QP_SLEEP_MS_INITIAL;
    pr_devel(DIS_STATUS_START);

    diswq->dismsq.l_qpn         = diswq->l_qpn;
    diswq->dismsq.r_qpn         = diswq->r_qpn;
    diswq->dismsq.max_msg_count = 16;
    diswq->dismsq.max_msg_size  = 128;
    diswq->dismsq.timeout       = 1234;
    diswq->dismsq.flags         = 0;
    
    while (!kthread_should_stop()) {
        ret = sci_if_create_msq(&diswq->dismsq);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }
        sleep_ms = min(sleep_ms + DIS_QP_SLEEP_MS_INCREASE, DIS_QP_SLEEP_MS_MAX);
        msleep(sleep_ms);
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_rq_post(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_rq_exit(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    sci_if_remove_msq(&diswq->dismsq);

    pr_devel(DIS_STATUS_COMPLETE);
}

int dis_sq_init(struct dis_wq *diswq)
{
    int ret, sleep_ms = DIS_QP_SLEEP_MS_INITIAL;
    pr_devel(DIS_STATUS_START);

    diswq->dismsq.l_qpn         = diswq->l_qpn;
    diswq->dismsq.r_qpn         = diswq->r_qpn;
    diswq->dismsq.max_msg_count = 16;
    diswq->dismsq.max_msg_size  = 128;
    diswq->dismsq.timeout       = 1234;
    diswq->dismsq.flags         = 0;

    while (!kthread_should_stop()) {
        ret = sci_if_connect_msq(&diswq->dismsq);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }
        sleep_ms = min(sleep_ms + 10, DIS_QP_SLEEP_MS_MAX);
        msleep(sleep_ms);
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_sq_post(struct dis_wq *diswq)
{
    int ret, size_free;
    struct dis_wqe *sqe;
    struct sci_if_msg msg;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        sqe = &diswq->wqe[diswq->wqe_get % diswq->max_wqe];

        if(sqe->flags != DIS_WQE_VALID) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }

        memset(&msg, 0, sizeof(struct sci_if_msg));
        msg.msq     = &diswq->dismsq.msq;
        msg.msg     = (void *)(sqe->sg_list[0].addr); //TODO: Include all segments
        msg.size    = sqe->sg_list[0].length;
        msg.free    = &size_free;
        msg.flags   = 0; //SCIL_FLAG_SEND_RECEIVE_PAIRS_ONLY

        // while (!kthread_should_stop()) {
        //     ret = sci_if_send_request(&msg);
        //     //TODO: Introduce error codes to exit on lost connection
        //     if (!ret) {
        //         break;
        //     }
        // }

        sqe->flags = DIS_WQE_FREE;
        diswq->wqe_get++;
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

void dis_sq_exit(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);

    sci_if_disconnect_msq(&diswq->dismsq);

    pr_devel(DIS_STATUS_COMPLETE);
}

int dis_wq_thread(void *diswq_buf)
{
    int ret, signal;
    bool exit = false;
    struct dis_wq *diswq = (struct dis_wq*)diswq_buf;
    pr_devel(DIS_STATUS_START);
    
    diswq->wq_state = DIS_WQ_RUNNING;

    switch (diswq->wq_type) {
    case DIS_RQ:
        ret = dis_rq_init(diswq);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            exit = true;
        }
        break;

    case DIS_SQ:
        ret = dis_sq_init(diswq);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            exit = true;
        }
        break;

    default:
        pr_devel("WQ Type: Unknown, exiting!");
        exit = true;
        break;
    }
 
    while (!exit && !kthread_should_stop()) {
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

        switch (diswq->wq_type) {
        case DIS_RQ:
            ret = dis_rq_post(diswq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;
        case DIS_SQ:
            ret = dis_sq_post(diswq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;
        default:
            pr_devel("WQ Type: Unknown, exiting!");
            exit = true;
            break;
        }

        diswq->wq_flag = DIS_WQ_EMPTY;
    }

    switch (diswq->wq_type) {
    case DIS_RQ:
        dis_sq_exit(diswq);
        break;

    case DIS_SQ:
        dis_rq_exit(diswq);
        break;

    default:
        pr_devel("WQ Type: Unknown, exiting!");
        break;
    }

    diswq->wq_flag  = DIS_WQ_EMPTY;
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

    if (diswq->wq_state != DIS_WQ_RUNNING) {
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

    pr_devel(DIS_STATUS_COMPLETE);
}