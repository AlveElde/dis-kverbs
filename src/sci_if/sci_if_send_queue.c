#include "pr_fmt.h"

#include "sci_if_send_queue.h"
#include "sci_if_lib.h"
#include "../driver/dis_verbs.h"

static int sci_if_process_sqe(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);


    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void sci_if_sq_signal(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);

    wake_up(&disqp->sq.wait_queue);

    pr_devel(DIS_STATUS_COMPLETE);
}
EXPORT_SYMBOL(sci_if_sq_signal);

static int sci_if_sq_thread(void *disqp_data)
{
    int ret, signal, safety = 0;
    bool exit = false;
    struct dis_qp *disqp = (struct dis_qp*)disqp_data;
    pr_devel(DIS_STATUS_START);
 
    while(exit || safety < 10) {
        signal = wait_event_killable(disqp->sq.wait_queue,
                                        disqp->sq.thread_flag != SCI_IF_EMPTY);
        if(signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        switch (disqp->sq.thread_flag)
        {
        case SCI_IF_POST_SEND:
            pr_devel("WQ Flag: SCI_IF_POST_SEND");
            ret = sci_if_process_sqe(disqp);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                exit = true;
            }
            break;

        case SCI_IF_EXIT:
            pr_devel("WQ Flag: SCI_IF_EXIT");
            exit = true;
            break;
        
        default:
            pr_devel("WQ Flag: Unknown, exiting!");
            exit = true;
            break;
        }

        disqp->sq.thread_flag = SCI_IF_EMPTY;
        safety++;
    }

    if(safety >= 10) {
        pr_devel("Safety tripped");
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static int sci_if_sq_init(struct dis_qp *disqp)
{
    pr_devel(DIS_STATUS_START);

    init_waitqueue_head(&disqp->sq.wait_queue);
    disqp->sq.thread = kthread_create(sci_if_sq_thread, (void*)disqp, "SQ Thread");
    if (!disqp->sq.thread) {
        pr_devel(DIS_STATUS_FAIL);
        return -1;
    }
    
    wake_up_process(disqp->sq.thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}
EXPORT_SYMBOL(sci_if_sq_init);


// static int sci_if_init_rq(struct dis_qp *disqp)
// {
//     pr_devel(DIS_STATUS_START);
    
//     init_waitqueue_head(&rq_wait_queue);
//     disqp->rq.thread = kthread_create(sci_if_rq_thread, (void*)disqp, "RQ Thread");
//     if (!disqp->rq.thread) {
//         pr_devel(DIS_STATUS_FAIL);
//         return -1;
//     }

//     wake_up_process(rq_thread);

//     pr_devel(DIS_STATUS_COMPLETE);
//     return 0;
// }

// static int sci_if_init_qp(struct dis_qp *disqp)
// {
//     pr_devel(DIS_STATUS_START);

//     pr_devel(DIS_STATUS_COMPLETE);
//     return 0;
// }

