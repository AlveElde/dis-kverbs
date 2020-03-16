#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"

int dis_sqe_execute(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rqe_execute(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_sq_connect(struct dis_wq *diswq)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rq_connect(struct dis_wq *diswq)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_wq_thread(void *diswq_data)
{
    int ret, signal;
    bool exit = false;
    struct dis_wq *diswq = (struct dis_wq*)diswq_data;
    pr_devel(DIS_STATUS_START);
    
    diswq->thread_status = DIS_WQ_RUNNING;
 
    while(!exit) {
        signal = wait_event_killable(diswq->wait_queue,
                                        diswq->thread_flag != DIS_WQ_EMPTY);
        if(signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        //TODO: Mutex
        switch (diswq->thread_flag) {
        case DIS_WQ_POST_SEND:
            pr_devel("Thread flag: DIS_WQ_POST_SEND");
            ret = dis_sqe_execute(diswq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;

        case DIS_WQ_POST_RECV:
            pr_devel("Thread flag: DIS_WQ_POST_RECV");
            ret = dis_rqe_execute(diswq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;

        case DIS_WQ_EXIT:
            pr_devel("Thread flag: DIS_WQ_EXIT");
            exit = true;
            break;
        
        default:
            pr_devel("Thread flag: Unknown, exiting!");
            exit = true;
            break;
        }

        diswq->thread_flag = DIS_WQ_EMPTY;
    }

    diswq->thread_flag      = DIS_WQ_EMPTY;
    diswq->thread_status    = DIS_WQ_EXITED;
    pr_devel(DIS_STATUS_COMPLETE);
    do_exit(0);
    return 0;
}

int dis_wq_signal(struct dis_wq *diswq, enum dis_wq_flag flag)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (diswq->thread_status != DIS_WQ_RUNNING) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    diswq->thread_flag = flag;
    wake_up(&diswq->wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_wq_init(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex
    
    init_waitqueue_head(&diswq->wait_queue);

    diswq->thread_status = DIS_WQ_INITIALIZED;

    diswq->thread = kthread_create(dis_wq_thread, (void*)diswq, "DIS WQ Thread");//TODO: More descriptive name
    if (!diswq->thread) {
        pr_devel(DIS_STATUS_FAIL);
        diswq->thread_status = DIS_WQ_UNINITIALIZED;
        return -42;
    }
    
    wake_up_process(diswq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_wq_exit(struct dis_wq *diswq)
{
    int sleep_ms = 0;
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (diswq->thread_status == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_COMPLETE);
        return;
    }

    while (diswq->thread_status != DIS_WQ_EXITED) {
        dis_wq_signal(diswq, DIS_WQ_EXIT);
        msleep(sleep_ms);
        sleep_ms = min(sleep_ms + 1, DIS_QP_SLEEP_MS_MAX);
    }

    pr_devel(DIS_STATUS_COMPLETE);
}

int dis_qp_init(struct dis_qp *disqp)
{
    int ret;
    pr_devel(DIS_STATUS_START);

    disqp->sq.thread_status = DIS_WQ_UNINITIALIZED;
    disqp->sq.thread_flag   = DIS_WQ_EMPTY;

    disqp->rq.thread_status = DIS_WQ_UNINITIALIZED;
    disqp->rq.thread_flag   = DIS_WQ_EMPTY;

    ret = dis_wq_init(&disqp->sq);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    ret = dis_wq_init(&disqp->rq);
    if (ret) {
        dis_wq_exit(&disqp->sq);
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_qp_exit(struct dis_qp *disqp)
{
    int ret;
    pr_devel(DIS_STATUS_START);

    dis_wq_exit(&disqp->sq);
    dis_wq_exit(&disqp->rq);

    pr_devel(DIS_STATUS_COMPLETE);
}