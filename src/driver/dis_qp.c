#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"
#include "dis_sci_if.h"

//TODO: Separated for FTrace, combine with dis_wq_post_rqe_cqe
int dis_wq_post_sqe_cqe(struct dis_wq *wq, 
                    struct dis_wqe *wqe,
                    enum ib_wc_status wq_status) 
{
    unsigned long flags, head, tail;
    struct dis_cq* cq = wq->cq;
    struct dis_cqe* cqe;
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&cq->cqe_lock, flags);

    /* Check that circular buffer is not full */
    head = cq->cqe_circ.head;
    tail = READ_ONCE(cq->cqe_circ.tail);
    if(CIRC_SPACE(head, tail, cq->cqe_max) < 1) {
        pr_devel(DIS_STATUS_FAIL);
        spin_unlock_irqrestore(&cq->cqe_lock, flags);
        return -42;
    }
    cqe = (struct dis_cqe*)&cq->cqe_circ.buf[head * sizeof(struct dis_cqe)];

    cqe->wr_id      = wqe->wr_id;
    cqe->opcode     = wqe->opcode;
    cqe->byte_len   = wqe->byte_len;
    cqe->ibqp       = wqe->ibqp;
    cqe->status     = wq_status;
    
    /* Advance the head of the circular buffer */
    smp_store_release(&cq->cqe_circ.head, (head + 1) & (cq->cqe_max - 1));

    spin_unlock_irqrestore(&cq->cqe_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_wq_post_rqe_cqe(struct dis_wq *wq, 
                    struct dis_wqe *wqe,
                    enum ib_wc_status wq_status) 
{
    unsigned long flags, head, tail;
    struct dis_cq* cq = wq->cq;
    struct dis_cqe* cqe;
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&cq->cqe_lock, flags);

    /* Check that circular buffer is not full */
    head = cq->cqe_circ.head;
    tail = READ_ONCE(cq->cqe_circ.tail);
    if(CIRC_SPACE(head, tail, cq->cqe_max) < 1) {
        pr_devel(DIS_STATUS_FAIL);
        spin_unlock_irqrestore(&cq->cqe_lock, flags);
        return -42;
    }
    cqe = (struct dis_cqe*)&cq->cqe_circ.buf[head * sizeof(struct dis_cqe)];

    cqe->wr_id      = wqe->wr_id;
    cqe->opcode     = wqe->opcode;
    cqe->byte_len   = wqe->byte_len;
    cqe->ibqp       = wqe->ibqp;
    cqe->status     = wq_status;
    
    /* Advance the head of the circular buffer */
    smp_store_release(&cq->cqe_circ.head, (head + 1) & (cq->cqe_max - 1));

    spin_unlock_irqrestore(&cq->cqe_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

enum ib_wc_status dis_wq_consume_one_rqe(struct dis_wq *wq, struct dis_wqe *wqe)
{
    int ret, bytes_left, bytes_total, fail_counter = 0;
    pr_devel(DIS_STATUS_START);

    bytes_total = wqe->byte_len;
    bytes_left = wqe->byte_len;
    while (!kthread_should_stop()) {
        ret = dis_sci_if_receive_v_msg(wqe);
        if (!ret) {
            bytes_left -= wqe->byte_len;
            // Note: wqe->byte_len is only changed on a successful, but partial, receive
            if(bytes_left == 0) {
                pr_devel(DIS_STATUS_COMPLETE);
                wqe->byte_len = bytes_total;
                dis_wq_post_rqe_cqe(wq, wqe, IB_WC_SUCCESS);
                return IB_WC_SUCCESS;
            }
            pr_devel("Partial receive!");
        }
        wqe->byte_len = bytes_left;
        
        //TODO: Replace sleep by returning when connection is broken
        if(fail_counter > 1000) {
            udelay(1);
        } else {
            fail_counter++;
        }
    }

    dis_wq_post_rqe_cqe(wq, wqe, IB_WC_RESP_TIMEOUT_ERR);

    pr_devel(DIS_STATUS_FAIL);
    return IB_WC_RESP_TIMEOUT_ERR;
}

enum ib_wc_status dis_wq_consume_one_sqe(struct dis_wq *wq, struct dis_wqe *wqe)
{
    int ret, fail_counter = 0;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        ret = dis_sci_if_send_v_msg(wqe);
        if (!ret) {
            pr_devel(DIS_STATUS_COMPLETE);
            dis_wq_post_sqe_cqe(wq, wqe, IB_WC_SUCCESS);
            return IB_WC_SUCCESS;
        }
        
        //TODO: Replace sleep by returning when connection is broken
        if(fail_counter > 1000) {
            udelay(1);
        } else {
            fail_counter++;
        }
    }
    
    dis_wq_post_sqe_cqe(wq, wqe, IB_WC_RESP_TIMEOUT_ERR);
    pr_devel(DIS_STATUS_FAIL);
    return IB_WC_RESP_TIMEOUT_ERR;
}

int dis_wq_consume_all(struct dis_wq *wq)
{
    unsigned long head, tail;
    struct dis_wqe *wqe;
    enum ib_wc_status wc_status;
    pr_devel(DIS_STATUS_START);

    while (!kthread_should_stop()) {
        /* Check if circular buffer is empty */
        head = smp_load_acquire(&wq->wqe_circ.head);
        tail = wq->wqe_circ.tail;
        if(CIRC_CNT(head, tail, wq->wqe_max) < 1) {
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }
        wqe = (struct dis_wqe*)&wq->wqe_circ.buf[tail * sizeof(struct dis_wqe)];

        switch (wq->wq_type) {
        case DIS_RQ:
            wc_status = dis_wq_consume_one_rqe(wq, wqe);
            break;

        case DIS_SQ:
            wc_status = dis_wq_consume_one_sqe(wq, wqe);
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        //TODO: Reintroduce:
        // dis_wq_post_cqe(wq, wqe, wc_status);

        if (wqe->page_map_dynamic) {
            kfree(wqe->page_map_dynamic);
        }

        /* Advance the tail of the circular buffer */
        smp_store_release(&wq->wqe_circ.tail, (tail + 1) & (wq->wqe_max - 1));
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
        
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
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
    int ret;
    struct dis_wq *wq = (struct dis_wq*)wq_buf;
    pr_devel(DIS_STATUS_START);
    
    /* Initialize connection to remote QP */
    wq->wq_state = DIS_WQ_RUNNING;
    ret = dis_wq_init(wq);
    if (ret) {
        wq->wq_state = DIS_WQ_EXITED;
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    
    while (!kthread_should_stop()) {
        /* Process all new work requests */
        wq->wq_flag = DIS_WQ_EMPTY;
        ret = dis_wq_consume_all(wq);
        if (ret) {
            break;
        }

        /* Wait for new work requests to be posted */
        ret = wait_event_killable(wq->wait_queue, wq->wq_flag != DIS_WQ_EMPTY ||
                                    kthread_should_stop());
        if (ret) {
            pr_devel("Kill signal received, exiting!");
            break;
        }
    }

    /* Tear down connection to remote QP */
    dis_wq_exit(wq);
    wq->wq_state = DIS_WQ_EXITED;

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_init(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);

    if(wq->wq_state == DIS_WQ_EXITED) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    
    /* Initialize wait queue */
    init_waitqueue_head(&wq->wait_queue);
    wq->wq_state = DIS_WQ_INITIALIZED;
    wq->wq_flag  = DIS_WQ_EMPTY;

    /* Create worker thread */
    wq->thread = kthread_create(dis_wq_thread, (void*)wq, "DIS WQ Thread");//TODO: More descriptive name
    if (!wq->thread) {
        pr_devel(DIS_STATUS_FAIL);
        wq->wq_state = DIS_WQ_UNINITIALIZED;
        return -42;
    }
    
    /* Start worker thread */
    wake_up_process(wq->thread);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_post_one_sqe(struct dis_wq *wq,
                        const struct ib_send_wr *wr)
{
    int i;
    u64 sge_chunk, head, tail;
    struct dis_pd *pd = to_dis_pd(wq->ibqp->pd);
    struct dis_wqe *wqe;
    struct dis_mr *mr;
    struct dis_sge_map sge_map[DIS_SGE_PER_WQE], *sge;
    struct ib_sge *ibsge;
    struct iovec *sge_page, *mr_page;
    pr_devel(DIS_STATUS_START);

    /* Check that circular buffer is not full */
    head = wq->wqe_circ.head;
    tail = READ_ONCE(wq->wqe_circ.tail);
    if(CIRC_SPACE(head, tail, wq->wqe_max) < 1) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    wqe = (struct dis_wqe*)&wq->wqe_circ.buf[head * sizeof(struct dis_wqe)];

    /* Set WQE attributes */
    wqe->opcode         = IB_WC_SEND;
    wqe->sci_msq        = &wq->sci_msq;
    wqe->byte_len       = 0;
    wqe->ibqp           = wq->ibqp;
    wqe->wr_id          = wr->wr_id;
    wqe->sci_msg.iovlen = 0;

    /* Calculate the number of pages spanned by each segment */
    for (i = 0; i < min(wr->num_sge, DIS_SGE_PER_WQE); i++) {
        ibsge = &wr->sg_list[i];
        sge = &sge_map[i];

        /* Retrieve MR for this SGE based on l_key */
        mr = pd->mr_list[ibsge->lkey];
        if (!mr) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        /* Memory can be mapped one-to-one for DMA MRs */
        if (mr->is_dma) {
            sge->is_dma         = 1;
            sge_page            = &wqe->page_map_static[i];
            sge_page->iov_base  = (void *)(ibsge->addr);
            sge_page->iov_len   = (size_t)ibsge->length;
            wqe->byte_len       += ibsge->length;
            wqe->sci_msg.iov    = sge_page;
            wqe->sci_msg.iovlen++;
            continue;
        }

        sge->is_dma         = 0;
        sge->mr_pages       = mr->pages;
        sge->sge_len        = ibsge->length;
        sge->base_offset    = (ibsge->addr - mr->mr_va) + mr->mr_va_offset;
        sge->page_offset    = sge->base_offset / PAGE_SIZE;
        sge->base_offset    -= sge->page_offset * PAGE_SIZE;
        sge->mr_pages       += sge->page_offset;
        sge->page_count     =
            ((u64)(sge->sge_len + sge->base_offset) / PAGE_SIZE) + 1;
        wqe->sci_msg.iovlen  += sge->page_count;
    }

    /* Choose store for the WQE page iovec map */
    if (wqe->sci_msg.iovlen > DIS_MAX_IOVLEN) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    } else if(wqe->sci_msg.iovlen <= DIS_SGE_PER_WQE) {
        wqe->sci_msg.iov = wqe->page_map_static;
    } else {
        wqe->page_map_dynamic = kmalloc(sizeof(struct iovec)*wqe->sci_msg.iovlen, 
                                        GFP_KERNEL);
        if (!wqe->page_map_dynamic) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }
        wqe->sci_msg.iov = wqe->page_map_dynamic;
    }
    
    /* Map each segment into physical pages from the MR */
    sge_page = wqe->sci_msg.iov;
    for (i = 0; i < min(wr->num_sge, DIS_SGE_PER_WQE); i++) {
        sge = &sge_map[i];
        
        if (sge->is_dma) {
            continue;
        }

        mr_page = sge->mr_pages;
        while (sge->sge_len > 0) {
            sge_chunk = min(sge->sge_len, PAGE_SIZE - sge->base_offset);
            sge_page->iov_base  = mr_page->iov_base + sge->base_offset;
            sge_page->iov_len   = (size_t)sge_chunk;
            wqe->byte_len       += sge_chunk;
            sge->sge_len        -= sge_chunk;
            sge->base_offset    = 0;
            sge_page++;
            mr_page++;
        }
    }

    /* Advance the head of the circular buffer */
    smp_store_release(&wq->wqe_circ.head, (head + 1) & (wq->wqe_max - 1));
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_qp_post_one_rqe(struct dis_wq *wq,
                    const struct ib_recv_wr *wr)
{
    int i;
    u64 sge_chunk, head, tail;
    struct dis_pd *pd = to_dis_pd(wq->ibqp->pd);
    struct dis_wqe *wqe;
    struct dis_mr *mr;
    struct dis_sge_map sge_map[DIS_SGE_PER_WQE], *sge;
    struct ib_sge *ibsge;
    struct iovec *sge_page, *mr_page;
    pr_devel(DIS_STATUS_START);

    /* Check that circular buffer is not full */
    head = wq->wqe_circ.head;
    tail = READ_ONCE(wq->wqe_circ.tail);
    if(CIRC_SPACE(head, tail, wq->wqe_max) < 1) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    wqe = (struct dis_wqe*)&wq->wqe_circ.buf[head * sizeof(struct dis_wqe)];

    /* Set SQE attributes */
    wqe->opcode         = IB_WC_RECV;
    wqe->sci_msq        = &wq->sci_msq;
    wqe->byte_len       = 0;
    wqe->ibqp           = wq->ibqp;
    wqe->wr_id          = wr->wr_id;
    wqe->sci_msg.iovlen = 0;

    /* Map each segment(SGE) into physical pages from the MR */
    for (i = 0; i < min(wr->num_sge, DIS_SGE_PER_WQE); i++) {
        ibsge = &wr->sg_list[i];
        sge = &sge_map[i];

        /* Retrieve MR for this SGE based on l_key */
        mr = pd->mr_list[ibsge->lkey];
        if (!mr) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        /* Memory can be mapped one-to-one for DMA MRs */
        if (mr->is_dma) {
            sge->is_dma         = 1;
            sge_page            = &wqe->page_map_static[i];
            sge_page->iov_base  = (void *)(ibsge->addr);
            sge_page->iov_len   = (size_t)ibsge->length;
            wqe->byte_len       += ibsge->length;
            wqe->sci_msg.iov    = sge_page;
            wqe->sci_msg.iovlen++;
            continue;
        }

        sge->is_dma         = 0;
        sge->mr_pages       = mr->pages;
        sge->sge_len        = ibsge->length;
        sge->base_offset    = (ibsge->addr - mr->mr_va) + mr->mr_va_offset;
        sge->page_offset    = sge->base_offset / PAGE_SIZE;
        sge->base_offset    -= sge->page_offset * PAGE_SIZE;
        sge->mr_pages       += sge->page_offset;
        sge->page_count     =
            ((u64)(sge->sge_len + sge->base_offset) / PAGE_SIZE) + 1;
        wqe->sci_msg.iovlen  += sge->page_count;
    }

    /* Choose store for the WQE page iovec map */
    if (wqe->sci_msg.iovlen > DIS_MAX_IOVLEN) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    } else if(wqe->sci_msg.iovlen <= DIS_SGE_PER_WQE) {
        wqe->sci_msg.iov = wqe->page_map_static;
    } else {
        wqe->page_map_dynamic = kmalloc(sizeof(struct iovec)*wqe->sci_msg.iovlen, 
                                        GFP_KERNEL);
        if (!wqe->page_map_dynamic) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }
        wqe->sci_msg.iov = wqe->page_map_dynamic;
    }
    
    /* Fill WQE iovec page map with MR iovecs, and offset pages on SGE bounds */
    sge_page = wqe->sci_msg.iov;
    for (i = 0; i < min(wr->num_sge, DIS_SGE_PER_WQE); i++) {
        sge = &sge_map[i];
        mr_page = sge->mr_pages;
        
        if (sge->is_dma) {
            continue;
        }

        while (sge->sge_len > 0) {
            sge_chunk = min(sge->sge_len, PAGE_SIZE - sge->base_offset);
            sge_page->iov_base  = mr_page->iov_base + sge->base_offset;
            sge_page->iov_len   = (size_t)sge_chunk;
            wqe->byte_len       += sge_chunk;
            sge->sge_len        -= sge_chunk;
            sge->base_offset    = 0;
            sge_page++;
            mr_page++;
        }
    }

    /* Advance the head of the circular buffer */
    smp_store_release(&wq->wqe_circ.head, (head + 1) & (wq->wqe_max - 1));
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

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_qp_exit(struct dis_wq *wq)
{
    pr_devel(DIS_STATUS_START);

    if (wq->wq_state == DIS_WQ_UNINITIALIZED || wq->wq_state == DIS_WQ_EXITED) {
        pr_devel(DIS_STATUS_COMPLETE);
        return;
    }

    /* Command the worker thread to stop and wait for it to exit */
    kthread_stop(wq->thread);
    wake_up(&wq->wait_queue);
    while (wq->wq_state != DIS_WQ_EXITED) {
        msleep(1);
    }

    pr_devel(DIS_STATUS_COMPLETE);
}