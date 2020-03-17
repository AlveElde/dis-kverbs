#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"
#include "sci_if.h"

int dis_sq_post(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rq_post(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_sq_create(struct dis_wq *diswq)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    // ret = sci_if_create_msq(&diswq->dismsq);
    // if (ret) {
    //     pr_devel(DIS_STATUS_FAIL);
    //     return -42;
    // }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rq_connect(struct dis_wq *diswq)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    // ret = sci_if_connect_msq(&diswq->dismsq);
    // if (ret) {
    //     pr_devel(DIS_STATUS_FAIL);
    //     return -42;
    // }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_sq_remove(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    // sci_if_disconnect_msq(&diswq->dismsq);

    pr_devel(DIS_STATUS_COMPLETE);
}

void dis_rq_disconnect(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    // sci_if_disconnect_msq(&diswq->dismsq);

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
    case DIS_SQ:
        ret = dis_sq_create(diswq);
        if(ret) {
            pr_devel(DIS_STATUS_FAIL);
            exit = true;
        }
        break;

    case DIS_RQ:
        ret = dis_rq_connect(diswq);
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
        case DIS_SQ:
            ret = dis_sq_post(diswq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;
        case DIS_RQ:
            ret = dis_rq_post(diswq);
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
    case DIS_SQ:
        dis_sq_remove(diswq);
        break;

    case DIS_RQ:
        dis_rq_disconnect(diswq);
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

int dis_wq_signal(struct dis_wq *diswq, enum dis_wq_flag flag)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (diswq->wq_state != DIS_WQ_RUNNING) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    diswq->wq_flag = flag;
    wake_up(&diswq->wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_wq_init(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex
    
    init_waitqueue_head(&diswq->wait_queue);

    diswq->wq_state = DIS_WQ_INITIALIZED;
    diswq->thread   = kthread_create(dis_wq_thread, (void*)diswq, "DIS WQ Thread");//TODO: More descriptive name
    if (!diswq->thread) {
        pr_devel(DIS_STATUS_FAIL);
        diswq->wq_state = DIS_WQ_UNINITIALIZED;
        return -42;
    }
    
    wake_up(&diswq->wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_wq_exit(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (diswq->wq_state == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_COMPLETE);
        return;
    }

    kthread_stop(diswq->thread);
    wake_up(&diswq->wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
}

// int dis_qp_signal(struct dis_wq *disqp, enum dis_wq_flag flag)
// {
//     //TODO:
//     return 0;
// }

// int dis_qp_init(struct dis_qp *disqp)
// {
//     int ret;
//     pr_devel(DIS_STATUS_START);

//     disqp->sq.wq_state  = DIS_WQ_UNINITIALIZED;
//     disqp->sq.wq_flag   = DIS_WQ_EMPTY;
//     disqp->sq.wq_type   = DIS_SQ;

//     disqp->rq.wq_state  = DIS_WQ_UNINITIALIZED;
//     disqp->rq.wq_flag   = DIS_WQ_EMPTY;
//     disqp->rq.wq_type   = DIS_RQ;

//     ret = dis_wq_init(&disqp->sq);
//     if (ret) {
//         pr_devel(DIS_STATUS_FAIL);
//         return -42;
//     }

//     ret = dis_wq_init(&disqp->rq);
//     if (ret) {
//         dis_wq_exit(&disqp->sq);
//         pr_devel(DIS_STATUS_FAIL);
//         return -42;
//     }

//     pr_devel(DIS_STATUS_COMPLETE);
//     return 0;
// }

// void dis_qp_exit(struct dis_qp *disqp)
// {
//     int ret;
//     pr_devel(DIS_STATUS_START);

//     dis_wq_exit(&disqp->sq);
//     dis_wq_exit(&disqp->rq);

//     pr_devel(DIS_STATUS_COMPLETE);
// }