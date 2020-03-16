#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"


/**
 *    SEND QUEUE FUNCTIONS  
**/

int dis_sqe_execute(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_sq_signal(struct dis_qp *disqp, enum dis_thread_flag flag)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (disqp->sq.thread_status != DIS_RUNNING) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    disqp->sq.thread_flag = flag;
    wake_up(&disqp->sq.wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_sq_thread(void *disqp_data)
{
    int ret, signal, safety = 0;
    bool exit = false;
    struct dis_qp *disqp = (struct dis_qp*)disqp_data;
    pr_devel(DIS_STATUS_START);

    disqp->sq.thread_status = DIS_RUNNING;
 
    while(exit || safety < 10) {
        signal = wait_event_killable(disqp->sq.wait_queue,
                                        disqp->sq.thread_flag != DIS_EMPTY);
        if(signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        switch (disqp->sq.thread_flag)
        {
        case DIS_POST_SEND:
            pr_devel("Thread flag: DIS_POST_SEND");
            ret = dis_sqe_execute(disqp);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;

        case DIS_EXIT:
            pr_devel("Thread flag: DIS_EXIT");
            exit = true;
            break;
        
        default:
            pr_devel("Thread flag: Unknown, exiting!");
            exit = true;
            break;
        }

        disqp->sq.thread_flag = DIS_EMPTY;
        safety++;
    }

    if(safety >= 10) {
        pr_devel("Safety tripped");
    }

    disqp->sq.thread_status = DIS_EXITED;
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_sq_init(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    init_waitqueue_head(&disqp->sq.wait_queue);
    disqp->sq.thread = kthread_create(dis_sq_thread, (void*)disqp, "SQ Thread");
    if (!disqp->sq.thread) {
        pr_devel(DIS_STATUS_FAIL);
        return -1;
    }
    
    wake_up_process(disqp->sq.thread);
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_sq_exit(struct dis_qp *disqp) 
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    dis_sq_signal(disqp, DIS_EXIT);

    while(disqp->sq.thread_status != DIS_EXITED)
    {
        msleep(10);
    }
    pr_devel(DIS_STATUS_COMPLETE);
}

/**
 *    RECEIVE QUEUE FUNCTIONS  
**/

int dis_rqe_execute(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rq_signal(struct dis_qp *disqp, enum dis_thread_flag flag)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (disqp->rq.thread_status != DIS_RUNNING) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    disqp->sq.thread_flag = flag;
    wake_up(&disqp->rq.wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rq_thread(void *disqp_data)
{
    int ret, signal, safety = 0;
    bool exit = false;
    struct dis_qp *disqp = (struct dis_qp*)disqp_data;
    pr_devel(DIS_STATUS_START);

    disqp->rq.thread_status = DIS_RUNNING;
 
    while(exit || safety < 10) {
        signal = wait_event_killable(disqp->rq.wait_queue,
                                        disqp->rq.thread_flag != DIS_EMPTY);
        if(signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        //TODO: Mutex
        switch (disqp->rq.thread_flag)
        {
        case DIS_POST_RECV:
            pr_devel("Thread flag: DIS_POST_RECV");
            ret = dis_rqe_execute(disqp);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;

        case DIS_EXIT:
            pr_devel("Thread flag: DIS_EXIT");
            exit = true;
            break;
        
        default:
            pr_devel("Thread flag: Unknown, exiting!");
            exit = true;
            break;
        }

        safety++;
    }

    disqp->rq.thread_flag = DIS_EMPTY;

    if(safety >= 10) {
        pr_devel("Safety tripped");
    }

    disqp->rq.thread_status = DIS_EXITED;
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_rq_init(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    
    init_waitqueue_head(&disqp->rq.wait_queue);

    disqp->rq.thread = kthread_create(dis_rq_thread, (void*)disqp, "RQ Thread");//TODO: More descriptive name
    if (!disqp->rq.thread) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    
    wake_up_process(disqp->rq.thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_rq_exit(struct dis_qp *disqp) 
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    dis_rq_signal(disqp, DIS_EXIT);

    while(disqp->rq.thread_status != DIS_EXITED)
    {
        msleep(10);
    }
    pr_devel(DIS_STATUS_COMPLETE);
}

// int dis_init_qp(struct dis_qp *disqp)
// {
//     pr_devel(DIS_STATUS_START);

//     pr_devel(DIS_STATUS_COMPLETE);
//     return 0;
// }

