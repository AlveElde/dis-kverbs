#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"


int dis_sqe_execute(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}


int dis_rqe_execute(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}


int dis_wq_signal(struct dis_wq *diswq, enum dis_thread_flag flag)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    if (diswq->thread_status != DIS_RUNNING) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    diswq->thread_flag = flag;
    wake_up(&diswq->wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_wq_thread(void *diswq_data)
{
    int ret, signal, safety = 0;
    bool exit = false;
    struct dis_wq *diswq = (struct dis_wq*)diswq_data;
    pr_devel(DIS_STATUS_START);

    diswq->thread_status = DIS_RUNNING;
 
    while(exit || safety < 10) {
        signal = wait_event_killable(diswq->wait_queue,
                                        diswq->thread_flag != DIS_EMPTY);
        if(signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        //TODO: Mutex
        switch (diswq->thread_flag)
        {
        case DIS_POST_WQE:
            pr_devel("Thread flag: DIS_POST_WQE");
            // ret = dis_rqe_execute(diswq);
            // if(ret) {
            //     pr_devel(DIS_STATUS_FAIL);
            //     exit = true;
            // }
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

    diswq->thread_flag = DIS_EMPTY;

    if (safety >= 10) {
        pr_devel("Safety tripped");
    }

    diswq->thread_status = DIS_EXITED;
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_wq_init(struct dis_wq *diswq)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex
    
    init_waitqueue_head(&diswq->wait_queue);

    diswq->thread = kthread_create(dis_wq_thread, (void*)diswq, "DIS WQ Thread");//TODO: More descriptive name
    if (!diswq->thread) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    
    wake_up_process(diswq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_wq_exit(struct dis_wq *diswq) 
{
    pr_devel(DIS_STATUS_START);
    //TODO: Mutex

    dis_wq_signal(diswq, DIS_EXIT);

    while(diswq->thread_status != DIS_EXITED)
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