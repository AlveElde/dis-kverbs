#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"
#include "dis_sci_if.h"

int dis_wq_post_cqe(struct dis_wq *wq, 
                    struct dis_wqe *wqe,
                    enum ib_wc_status wq_status) 
{
    unsigned long flags;
    struct dis_cq* cq = wq->cq;
    struct dis_cqe* cqe;
    pr_devel(DIS_STATUS_START);

    // TODO: Make an individual cqe lock?
    spin_lock_irqsave(&cq->cqe_lock, flags);

    cqe = cq->cqe_queue + (cq->cqe_put % cq->cqe_max);

    if(cqe->valid) {
        spin_unlock_irqrestore(&cq->cqe_lock, flags);
        return -42;
    }

    cqe->wr_id      = wqe->wr_id;
    cqe->opcode     = wqe->opcode;
    cqe->byte_len   = wqe->byte_len;
    cqe->ibqp       = wqe->ibqp;
    cqe->status     = wq_status;
    cqe->valid      = 1;

    cq->cqe_put++;

    spin_unlock_irqrestore(&cq->cqe_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

enum ib_wc_status dis_wq_consume_one_rqe(struct dis_wqe *wqe)
{
    int ret, bytes_left, bytes_total;
    pr_devel(DIS_STATUS_START);

    bytes_left = wqe->byte_len;
    bytes_total = wqe->byte_len;
    while (!kthread_should_stop()) {
        ret = dis_sci_if_receive_v_msg(wqe);
        if (!ret) {
            // Note: wqe->byte_len is only changed on a successful, but partial, receive
            bytes_left -= wqe->byte_len;
            if(bytes_left == 0) {
                pr_devel(DIS_STATUS_COMPLETE);
                wqe->byte_len = bytes_total;
                return IB_WC_SUCCESS;
            }
            wqe->byte_len = bytes_left;
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return IB_WC_RESP_TIMEOUT_ERR;
}

enum ib_wc_status dis_wq_consume_one_sqe(struct dis_wqe *wqe)
{
    int ret;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        ret = dis_sci_if_send_v_msg(wqe);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            return IB_WC_SUCCESS;
        }
    }

    pr_devel(DIS_STATUS_FAIL);
    return IB_WC_RESP_TIMEOUT_ERR;
}

int dis_wq_consume_all(struct dis_wq *wq)
{
    struct dis_wqe *wqe;
    enum ib_wc_status wc_status;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        wqe = wq->wqe_queue + (wq->wqe_get % wq->wqe_max);

        if(!wqe->valid) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }

        switch (wq->wq_type) {
        case DIS_RQ:
            wc_status = dis_wq_consume_one_rqe(wqe);
            break;

        case DIS_SQ:
            wc_status = dis_wq_consume_one_sqe(wqe);
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        dis_wq_post_cqe(wq, wqe, wc_status);
        wqe->valid = 0;
        wq->wqe_get++;
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_wq_init(struct dis_wq *wq)
{
    int ret, sleep_ms;
    pr_devel(DIS_STATUS_START);

    sleep_ms = DIS_QP_SLEEP_MS_INI;
    while (!kthread_should_stop()) {
        switch (wq->wq_type) {
        case DIS_RQ:
            ret = dis_sci_if_create_msq(wq);
            if(!ret) {
                pr_devel(DIS_STATUS_COMPLETE);
                return 0;
            }
            break;

        case DIS_SQ:
            ret = dis_sci_if_connect_msq(wq);
            if(!ret) {
                pr_devel(DIS_STATUS_COMPLETE);
                return 0;
            }
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        sleep_ms = min(sleep_ms + DIS_QP_SLEEP_MS_INC, DIS_QP_SLEEP_MS_MAX);
        msleep_interruptible(sleep_ms);
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_wq_exit(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);

    switch (wq->wq_type) {
    case DIS_RQ:
        dis_sci_if_remove_msq(wq);
        break;

    case DIS_SQ:
        dis_sci_if_disconnect_msq(wq);
        break;

    default:
        pr_devel(DIS_STATUS_FAIL);
        return;
    }

    pr_devel(DIS_STATUS_COMPLETE);
}

int dis_wq_thread(void *wq_buf)
{
    int ret, signal;
    struct dis_wq *wq = (struct dis_wq*)wq_buf;
    pr_devel(DIS_STATUS_START);
    
    wq->wq_state = DIS_WQ_RUNNING;
    ret = dis_wq_init(wq);
    if (ret) {
        wq->wq_state = DIS_WQ_EXITED;
        pr_devel(DIS_STATUS_FAIL);
        return 0;
    }
 
    while (!kthread_should_stop()) {
        ret = dis_wq_consume_all(wq);
        if (ret) {
            break;
        }

        wq->wq_flag = DIS_WQ_EMPTY;

        signal = wait_event_killable(wq->wait_queue,
                                        wq->wq_flag != DIS_WQ_EMPTY ||
                                        kthread_should_stop());
        if (signal) {
            pr_devel("Kill signal received, exiting!");
            break;
        }

        if (kthread_should_stop()) {
            pr_devel("Kernel thread should stop, exiting!");
            break;
        }
    }

    dis_wq_exit(wq);
    wq->wq_state = DIS_WQ_EXITED;
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_init(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);
    
    init_waitqueue_head(&wq->wait_queue);
    wq->wq_state = DIS_WQ_INITIALIZED;
    wq->wq_flag  = DIS_WQ_EMPTY;

    wq->thread = kthread_create(dis_wq_thread, (void*)wq, "DIS WQ Thread");//TODO: More descriptive name
    if (!wq->thread) {
        pr_devel(DIS_STATUS_FAIL);
        wq->wq_state = DIS_WQ_UNINITIALIZED;
        return -42;
    }
    
    wake_up_process(wq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_post_one_sqe(struct dis_wq *sq,
                        const struct ib_send_wr *send_wr)
{
    int i;
    u64 *page_pa, page_offset, sge_va, sge_length, sge_chunk;
    struct dis_pd *pd = to_dis_pd(sq->ibqp->pd);
    struct dis_wqe *sqe;
    struct dis_mr *mr;
    struct ib_sge *ibsge;
    struct iovec *iov;
    pr_devel(DIS_STATUS_START);

    sqe = sq->wqe_queue + (sq->wqe_put % sq->wqe_max);
    if (sqe->valid) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    sqe->valid      = 1;
    sqe->opcode     = IB_WC_SEND;
    sqe->sci_msq    = &sq->sci_msq;
    sqe->byte_len   = 0;
    sqe->ibqp       = sq->ibqp;
    sqe->wr_id      = send_wr->wr_id;

    sqe->sci_msg.cmsg_valid = 0;
    sqe->sci_msg.page       = NULL;
    sqe->sci_msg.iov        = sqe->iov;
    sqe->sci_msg.iovlen     = 0;

    for (i = 0; i < min(send_wr->num_sge, DIS_SGE_PER_WQE); i++) {
        ibsge   = &send_wr->sg_list[i];
        mr      = pd->mr_list[ibsge->lkey];
        if (!mr) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        sge_va      = (uintptr_t)(ibsge->addr);
        sge_length  = ibsge->length;
        page_pa     = mr->page_pa;
        page_offset = (sge_va - mr->mr_va) + mr->mr_va_offset;

        /* Find offset into the first page */
        while (page_offset >= DIS_PAGE_SIZE) {
            page_offset -= DIS_PAGE_SIZE;
            page_pa++;
        }

        /* Create IO Vectors for each page chunk */
        while (sge_length > 0 && sqe->sci_msg.iovlen < DIS_PAGE_PER_SGE) {
            sge_chunk   = min(sge_length, DIS_PAGE_SIZE - page_offset);
            iov         = &sqe->iov[sqe->sci_msg.iovlen];

            iov->iov_base   = (void *)(*page_pa + page_offset);
            iov->iov_len    = (size_t)sge_chunk;
            sqe->byte_len   += sge_chunk;

            sge_length -= sge_chunk;
            page_offset = 0;
            page_pa++;
            sqe->sci_msg.iovlen++;
        }
    }

    sq->wqe_put++;
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_post_one_rqe(struct dis_wq *rq,
                        const struct ib_recv_wr *recv_wr)
{ 
    int i;
    u64 *page_pa, page_offset, sge_va, sge_length, sge_chunk;
    struct dis_pd *pd = to_dis_pd(rq->ibqp->pd);
    struct dis_wqe *rqe;
    struct dis_mr *mr;
    struct ib_sge *ibsge;
    struct iovec *iov;
    pr_devel(DIS_STATUS_START);

    rqe = rq->wqe_queue + (rq->wqe_put % rq->wqe_max);
    if (rqe->valid) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    rqe->valid      = 1;
    rqe->opcode     = IB_WC_RECV;
    rqe->sci_msq    = &rq->sci_msq;
    rqe->byte_len   = 0;
    rqe->ibqp       = rq->ibqp;
    rqe->wr_id      = recv_wr->wr_id;

    rqe->sci_msg.cmsg_valid = 0;
    rqe->sci_msg.page       = NULL;
    rqe->sci_msg.iov        = rqe->iov;
    rqe->sci_msg.iovlen     = 0;

    for (i = 0; i < min(recv_wr->num_sge, DIS_SGE_PER_WQE); i++) {
        ibsge   = &recv_wr->sg_list[i];
        mr      = pd->mr_list[ibsge->lkey];
        if (!mr) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        sge_va      = (uintptr_t)(ibsge->addr);
        sge_length  = ibsge->length;
        page_pa     = mr->page_pa;
        page_offset = (sge_va - mr->mr_va) + mr->mr_va_offset;

        /* Find offset into the first page */
        while (page_offset >= DIS_PAGE_SIZE) {
            page_offset -= DIS_PAGE_SIZE;
            page_pa++;
        }

        /* Create IO Vectors for each page chunk */
        while (sge_length > 0 && rqe->sci_msg.iovlen < DIS_PAGE_PER_SGE) {
            sge_chunk   = min(sge_length, DIS_PAGE_SIZE - page_offset);
            iov         = &rqe->iov[rqe->sci_msg.iovlen];

            iov->iov_base   = (void *)(*page_pa + page_offset);
            iov->iov_len    = (size_t)sge_chunk;
            rqe->byte_len   += sge_chunk;

            sge_length -= sge_chunk;
            page_offset = 0;
            page_pa++;
            rqe->sci_msg.iovlen++;
        }
    }

    rq->wqe_put++;
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_notify(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);

    if (wq->wq_state == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    wq->wq_flag = DIS_WQ_POST;
    wake_up(&wq->wait_queue);
    //TODO: Should we yield here?

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_qp_exit(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);

    if (wq->wq_state == DIS_WQ_UNINITIALIZED) {
        pr_devel(DIS_STATUS_COMPLETE);
        return;
    }

    kthread_stop(wq->thread);
    wake_up(&wq->wait_queue);
    wake_up_process(wq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
}