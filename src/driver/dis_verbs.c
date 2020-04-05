#include "pr_fmt.h"

#include <rdma/ib_umem.h>

#include "dis_verbs.h"
#include "dis_qp.h"


int dis_alloc_ucontext(struct ib_ucontext *ibucontext, 
                                struct ib_udata *udata)
{
	// struct dis_ucontext *ucontext = to_dis_ucontext(ibucontext);
	// struct dis_dev *dev = to_dis_dev(ibucontext->device);
    pr_devel(DIS_STATUS_START);

    pr_devel(DIS_STATUS_COMPLETE);
	return 0;
}

void dis_dealloc_ucontext(struct ib_ucontext *ibucontext)
{
	// struct dis_ucontext *ucontext = to_dis_ucontext(ibucontext);
    pr_devel(DIS_STATUS_START);

    pr_devel(DIS_STATUS_COMPLETE);
}

int dis_query_device(struct ib_device *ibdev, struct ib_device_attr *dev_attr,
                        struct ib_udata *udata)
{
    pr_devel(DIS_STATUS_START);

    dev_attr->fw_ver               = 1;
    dev_attr->sys_image_guid       = 1234;
    dev_attr->max_mr_size          = 10000;
    dev_attr->page_size_cap        = DIS_PAGE_SIZE;
    dev_attr->vendor_id            = 0x1234;
    dev_attr->vendor_part_id       = 0x1234;
    dev_attr->hw_ver               = 1234;
    dev_attr->max_qp               = 1234;
    dev_attr->max_qp_wr            = 1234;
    dev_attr->device_cap_flags     = IB_DEVICE_ALLOW_USER_UNREG;
    dev_attr->device_cap_flags     |= IB_DEVICE_MEM_MGT_EXTENSIONS;
    dev_attr->max_send_sge         = DIS_SGE_PER_WQE;
    dev_attr->max_recv_sge         = DIS_SGE_PER_WQE;
    dev_attr->max_sge_rd           = 1;
    dev_attr->max_cq               = 1234;
    dev_attr->max_cqe              = 1234;
    dev_attr->max_mr               = DIS_MR_MAX;
    dev_attr->max_pd               = 1234;
    dev_attr->max_qp_rd_atom       = 0;
    dev_attr->max_qp_init_rd_atom  = 0;
    dev_attr->atomic_cap           = IB_ATOMIC_NONE;
    dev_attr->max_pkeys            = 1;
    dev_attr->local_ca_ack_delay   = 1;

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_query_port(struct ib_device *ibdev, u8 port,
                    struct ib_port_attr *port_attr)
{
    pr_devel(DIS_STATUS_START);

    port_attr->gid_tbl_len      = 1;
    port_attr->pkey_tbl_len     = 1;
    port_attr->max_vl_num       = 1;
    port_attr->max_msg_sz       = DIS_PAGE_SIZE * DIS_PAGE_PER_WQE;
    port_attr->max_mtu          = IB_MTU_4096;
    port_attr->active_mtu       = IB_MTU_4096;
    port_attr->lid              = 1234;
    port_attr->sm_lid           = 1234;
    port_attr->bad_pkey_cntr    = 0;
    port_attr->qkey_viol_cntr   = 0;
    port_attr->lmc              = 0;
    port_attr->sm_sl            = 0;
    port_attr->subnet_timeout   = 0;
    port_attr->init_type_reply  = 0;
    port_attr->phys_state       = IB_PORT_PHYS_STATE_LINK_UP;
    port_attr->state            = IB_PORT_ACTIVE;

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_get_port_immutable(struct ib_device *ibdev, u8 port_num,
                            struct ib_port_immutable *immutable)
{
    pr_devel(DIS_STATUS_START);
    //int ret;
    // struct ib_port_attr port_attr;
    // ret = dis_query_port(ibdev, port_num, &port_attr);
    // if(ret) {
    //     return -42;
    // }
    
    // TODO: Replace hard-coded values with result from dis_query_port
    immutable->pkey_tbl_len     = 1;
    immutable->gid_tbl_len      = 1;
    immutable->core_cap_flags   = RDMA_CORE_PORT_RAW_PACKET;
    
    // This has to be 0 in order to not trigger the verify_immutable check
    immutable->max_mad_size = 0; //IB_MGMT_MAD_SIZE;
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

enum rdma_link_layer dis_get_link_layer(struct ib_device *ibdev, u8 port_num)
{
	return IB_LINK_LAYER_UNSPECIFIED;
}

int dis_query_pkey(struct ib_device *ibdev, u8 port, u16 index,
                    u16 *pkey)
{
    pr_devel(DIS_STATUS_START);

    *pkey = 0xffff;
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;   
}

int dis_alloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    struct dis_pd *pd    = to_dis_pd(ibpd);
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *dev  = to_dis_dev(ibdev);
    pr_devel(DIS_STATUS_START);

    pd->dev = dev;
    pd->qp_c = DIS_QP_INI;
    pd->mr_c = DIS_MR_INI;

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_dealloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    pr_devel(DIS_STATUS_START);
    // Nothing to do.
    pr_devel(DIS_STATUS_COMPLETE);
}

struct ib_mr *dis_get_dma_mr(struct ib_pd *ibpd, int access)
{
    struct dis_mr *mr;
    pr_devel(DIS_STATUS_START);

    mr = kzalloc(sizeof(struct dis_mr), GFP_KERNEL);
    if (!mr) {
        pr_devel(DIS_STATUS_FAIL);
        return NULL;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return &mr->ibmr;
}

struct ib_mr *dis_reg_user_mr(struct ib_pd *ibpd, 
                                u64 start,
                                u64 length,
                                u64 va,
                                int access,
                                struct ib_udata *udata)
{
    u64 *page_pa;
    struct dis_mr *mr;
    struct dis_pd *pd = to_dis_pd(ibpd);
    struct sg_page_iter	sg;
    pr_devel(DIS_STATUS_START);
    
    if (pd->mr_c >= DIS_MR_MAX) {
        pr_devel(DIS_STATUS_FAIL);
        goto no_mr_left_err;
    }

    /* Allocate memory for MR structure */
    mr = kzalloc(sizeof(struct dis_mr), GFP_KERNEL);
    if (!mr) {
        pr_devel(DIS_STATUS_FAIL);
        goto mr_alloc_err;
    }

    /* Pin the page(s) containing the user segment */
    mr->ibumem = ib_umem_get(udata, start, length, access);
    if (IS_ERR(mr->ibumem)) {
        pr_devel(DIS_STATUS_FAIL);
        goto ib_umem_get_err;
    }

    /* Retrieve the number of pages pinned in previous step */
    mr->page_count = ib_umem_page_count(mr->ibumem);
    if (!mr->page_count) {
        pr_devel(DIS_STATUS_FAIL);
        goto page_count_err;
    }

    /* Allocate memory for a list of physical page addresses */
    mr->page_pa = kzalloc(sizeof(u64) * mr->page_count, GFP_KERNEL);
    if (!mr->page_pa) {
        pr_devel(DIS_STATUS_FAIL);
        goto page_pa_alloc_err;
    }

    /* Store the physical address of the start of each pinned page */
    page_pa = mr->page_pa;
    for_each_sg_page (mr->ibumem->sg_head.sgl, &sg, mr->ibumem->nmap, 0) {
        *page_pa = (uintptr_t)page_address(sg_page_iter_page(&sg));
        if (!(*page_pa)) {
            pr_devel(DIS_STATUS_FAIL);
            goto page_address_err;
        }
        page_pa++;
    }

    /* Store the virtual address start, length, and offset */
    mr->mr_va = va;
    mr->mr_length = length;
    mr->mr_va_offset = ib_umem_offset(mr->ibumem);

    /* Obtain an l_key and register this MR with PD */
    mr->ibmr.lkey = pd->mr_c;
    pd->mr_list[pd->mr_c] = mr;
    pd->mr_c++;

    pr_devel(DIS_STATUS_COMPLETE);
    return &mr->ibmr;

page_address_err:
    kfree(mr->page_pa);

page_pa_alloc_err:
page_count_err:
    ib_umem_release(mr->ibumem);

ib_umem_get_err:
    kfree(mr);

mr_alloc_err:
no_mr_left_err:
    return ERR_PTR(-EINVAL);
}

int dis_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
{
    struct dis_mr *mr = to_dis_mr(ibmr);
    pr_devel(DIS_STATUS_START);

    kfree(mr->page_pa);
    ib_umem_release(mr->ibumem);
    kfree(mr);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_create_cq(struct ib_cq *ibcq, const struct ib_cq_init_attr *init_attr,
                    struct ib_udata *udata)
{
    struct dis_cq *cq    = to_dis_cq(ibcq);
    struct ib_device *ibdev = ibcq->device;
    struct dis_dev *dev  = to_dis_dev(ibdev);
    pr_devel(DIS_STATUS_START);

    cq->dev         = dev;
    cq->cqe_get     = 0;
    cq->cqe_put     = 0;
    cq->cqe_max     = roundup_pow_of_two(init_attr->cqe);
    cq->cqe_queue   = kzalloc(sizeof(struct dis_cqe) * cq->cqe_max, 
                                GFP_KERNEL);
    if (!cq->cqe_queue) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    memset(cq->cqe_queue, 0, sizeof(struct dis_cqe) * cq->cqe_max);

    ibcq->cqe = cq->cqe_max;
    spin_lock_init(&cq->cqe_lock);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_poll_cq(struct ib_cq *ibcq, int num_wc, struct ib_wc *ibwc)
{
    int i, wc_count;
    struct dis_cq *cq = to_dis_cq(ibcq);
    struct dis_cqe* cqe;
    struct ib_wc *ibwc_iter;
    // pr_devel(DIS_STATUS_START);

    ibwc_iter = ibwc;
    wc_count = 0;
    for (i = 0; i < num_wc; i++) {

        cqe = cq->cqe_queue + (cq->cqe_get % cq->cqe_max);

        if(!cqe->valid) {
            break;
        }
        ibwc_iter->wr_id    = cqe->wr_id;
        ibwc_iter->status   = cqe->status;
        ibwc_iter->opcode   = cqe->opcode;
        ibwc_iter->byte_len = cqe->byte_len;
        ibwc_iter->qp       = cqe->ibqp;

        cqe->valid = 0;
        cq->cqe_get++;
        ibwc_iter++;
        wc_count++;
    }
    
    // pr_devel(DIS_STATUS_COMPLETE);
    return wc_count;
}

int dis_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
    pr_devel(DIS_STATUS_START);

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

void dis_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
{
    struct dis_cq *cq = to_dis_cq(ibcq);
    pr_devel(DIS_STATUS_START);

    kfree(cq->cqe_queue);

    pr_devel(DIS_STATUS_COMPLETE);
}

struct ib_qp *dis_create_qp(struct ib_pd *ibpd,
                            struct ib_qp_init_attr *init_attr,
                            struct ib_udata *udata)
{
    struct dis_qp *qp;
    struct dis_pd *pd = to_dis_pd(ibpd);
    pr_devel(DIS_STATUS_START);

    if (pd->qp_c >= DIS_QP_MAX) {
        pr_devel(DIS_STATUS_FAIL);
        goto no_qp_left_err;
    }

    /* Allocate memory for QP structure */
    qp = kzalloc(sizeof(struct dis_qp), GFP_KERNEL);
    if (!qp) {
        pr_devel(DIS_STATUS_FAIL);
        goto qp_alloc_err;
    }

    /* Set QP attributes */
    qp->dev             = to_dis_dev(ibpd->device);
    qp->sq_sig_type     = init_attr->sq_sig_type;
    qp->type            = init_attr->qp_type;
    qp->state           = IB_QPS_RESET;
    qp->mtu             = ib_mtu_enum_to_int(IB_MTU_4096);
    qp->l_qpn           = pd->qp_c;
    qp->event_handler   = init_attr->event_handler;

    qp->ibqp.pd         = ibpd;
    qp->ibqp.send_cq    = init_attr->send_cq;
    qp->ibqp.recv_cq    = init_attr->recv_cq;
    qp->ibqp.srq        = init_attr->srq;
    qp->ibqp.qp_type    = init_attr->qp_type;
    qp->ibqp.qp_num     = qp->l_qpn;

    /* Set SQ attributes*/
    qp->sq.ibqp         = &qp->ibqp;
    qp->sq.cq           = to_dis_cq(init_attr->send_cq);
    qp->sq.sge_max      = init_attr->cap.max_send_sge;
    qp->sq.inline_max   = init_attr->cap.max_inline_data;
    qp->sq.wqe_max      = roundup_pow_of_two(init_attr->cap.max_send_wr);
    qp->sq.wqe_get      = 0;
    qp->sq.wqe_put      = 0;
    qp->sq.wq_type      = DIS_SQ;
    
    /* Allocate memory for SQ */
    qp->sq.wqe_queue = kzalloc(sizeof(struct dis_wqe) * qp->sq.wqe_max, 
                                GFP_KERNEL);
    if (!qp->sq.wqe_queue) {
        pr_devel(DIS_STATUS_FAIL);
        goto create_qp_alloc_sq_err;
    }
    memset(qp->sq.wqe_queue, 0, sizeof(struct dis_wqe) * qp->sq.wqe_max);

    /* Set RQ attributes */
    qp->rq.ibqp         = &qp->ibqp;
    qp->rq.cq           = to_dis_cq(init_attr->recv_cq);
    qp->rq.sge_max      = init_attr->cap.max_recv_sge;
    qp->rq.inline_max   = init_attr->cap.max_inline_data;
    qp->rq.wqe_max      = roundup_pow_of_two(init_attr->cap.max_recv_wr);
    qp->rq.wqe_get      = 0;
    qp->rq.wqe_put      = 0;
    qp->rq.wq_type      = DIS_RQ;

    /* Allocate memory for RQ */
    qp->rq.wqe_queue = kzalloc(sizeof(struct dis_wqe) * qp->rq.wqe_max, 
                                GFP_KERNEL);
    if (!qp->rq.wqe_queue) {
        pr_devel(DIS_STATUS_FAIL);
        goto create_qp_alloc_rq_err;
    }
    memset(qp->rq.wqe_queue, 0, sizeof(struct dis_wqe) * qp->rq.wqe_max);

    /* Register QP with PD */
    pd->qp_list[pd->qp_c] = qp;
    pd->qp_c++;
    
    pr_devel(DIS_STATUS_COMPLETE);
    return &qp->ibqp;

create_qp_alloc_rq_err:
    kfree(qp->sq.wqe_queue);

create_qp_alloc_sq_err:
    kfree(qp);

qp_alloc_err:
no_qp_left_err:
    return NULL;
}

int dis_query_qp(struct ib_qp *ibqp, 
                    struct ib_qp_attr *attr,
                    int attr_mask,
                    struct ib_qp_init_attr *init_attr)
{
    pr_devel(DIS_STATUS_START);
    //TODO: Implement function.
    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_modify_qp(struct ib_qp *ibqp, 
                    struct ib_qp_attr *attr,
                    int attr_mask, 
                    struct ib_udata *udata)
{
    int ret;
    struct dis_qp *qp = to_dis_qp(ibqp);
    pr_devel(DIS_STATUS_START);

    if (attr_mask & IB_QP_STATE) {
        switch (attr->qp_state) {
        case IB_QPS_RESET:
            /* Transition QP state to RESET */
            pr_devel("Modify QP state: RESET");
            //TODO: Reset QP.
            break;

        case IB_QPS_INIT:
            /* Transition QP state to INIT */
            pr_devel("Modify QP state: INIT");
            //TODO: Handle init attrs.
            break;

        case IB_QPS_RTR:
            /* Transition QP state to RTR */
            pr_devel("Modify QP state: RTR");
            if (attr_mask & IB_QP_DEST_QPN) {
                // qp->r_qpn    = attr->dest_qp_num;
                qp->r_qpn       = qp->l_qpn;
    
                qp->rq.l_qpn    = qp->l_qpn;
                qp->rq.r_qpn    = qp->r_qpn;
                qp->sq.l_qpn    = qp->l_qpn;
                qp->sq.r_qpn    = qp->r_qpn;
            }
            
            if (attr_mask & IB_QP_PATH_MTU) {
                qp->mtu     = ib_mtu_enum_to_int(attr->path_mtu);
                qp->rq.mtu  = qp->mtu;
                qp->sq.mtu  = qp->mtu;
            }
            
            ret = dis_qp_init(&qp->rq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                return -42;
            }
            break;

        case IB_QPS_RTS:
            /* Transition QP state to RTS */
            pr_devel("Modify QP state: RTS");
            ret = dis_qp_init(&qp->sq);
            if(ret) {
                pr_devel(DIS_STATUS_FAIL);
                return -42;
            }
            break;

        default:
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        qp->state = attr->qp_state;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_destroy_qp(struct ib_qp *ibqp, struct ib_udata *udata)
{
    struct dis_qp *qp = to_dis_qp(ibqp);
    pr_devel(DIS_STATUS_START);

    dis_qp_exit(&qp->rq);
    dis_qp_exit(&qp->sq);
    kfree(qp->rq.wqe_queue);
    kfree(qp->sq.wqe_queue);
    kfree(qp);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_post_send(struct ib_qp *ibqp, 
                    const struct ib_send_wr *send_wr,
                    const struct ib_send_wr **bad_wr)
{
    int ret;
    struct dis_qp *qp = to_dis_qp(ibqp);
    const struct ib_send_wr *send_wr_iter;
    pr_devel(DIS_STATUS_START);

    send_wr_iter = send_wr;
    while (send_wr_iter) {
        ret = dis_qp_post_one_sqe(&qp->sq,
                                    send_wr_iter);
        if (ret) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        dis_qp_notify(&qp->sq);
        send_wr_iter = send_wr_iter->next;
    }
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_post_recv(struct ib_qp *ibqp, 
                    const struct ib_recv_wr *recv_wr,
                    const struct ib_recv_wr **bad_wr)
{
    int ret;
    struct dis_qp *qp = to_dis_qp(ibqp);
    const struct ib_recv_wr *recv_wr_iter;
    pr_devel(DIS_STATUS_START);

    recv_wr_iter = recv_wr;
    while (recv_wr_iter) {
        ret = dis_qp_post_one_rqe(&qp->rq,
                                    recv_wr_iter);
        if (ret) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        dis_qp_notify(&qp->rq);
        recv_wr_iter = recv_wr_iter->next;
    }
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_create_srq(struct ib_srq *ibsrq,
                    struct ib_srq_init_attr *init_attr, 
                    struct ib_udata *udata)
{   
    struct dis_srq *srq = to_dis_srq(ibsrq);
    struct ib_srq_attr *attr = &init_attr->attr;
    pr_devel(DIS_STATUS_START);

    srq->srq_type   = init_attr->srq_type;
    srq->srq_limit  = attr->srq_limit;

    // qp->rq.cq           = to_dis_cq(init_attr->recv_cq);
    srq->rq.sge_max     = attr->max_sge;
    srq->rq.wqe_get     = 0;
    srq->rq.wqe_put     = 0;
    srq->rq.wqe_max     = roundup_pow_of_two(attr->max_wr);
    srq->rq.wqe_queue   = kzalloc(sizeof(struct dis_wqe) * srq->rq.wqe_max, GFP_KERNEL);
    if (!srq->rq.wqe_queue) {
        pr_devel(DIS_STATUS_FAIL);
       return -42;
    }
    memset(srq->rq.wqe_queue, 0, sizeof(struct dis_wqe) * srq->rq.wqe_max);

    spin_lock_init(&srq->srq_lock);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_modify_srq(struct ib_srq *ibsrq, 
                    struct ib_srq_attr *attr, 
                    enum ib_srq_attr_mask attr_mask, 
                    struct ib_udata *udata)
{
    unsigned long flags;
    struct dis_srq *srq = to_dis_srq(ibsrq);
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&srq->srq_lock, flags);
    
    //TODO: Implement function.

    spin_unlock_irqrestore(&srq->srq_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}      

int dis_query_srq(struct ib_srq *ibsrq, struct ib_srq_attr *attr)
{
    unsigned long flags;
    struct dis_srq *srq = to_dis_srq(ibsrq);
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&srq->srq_lock, flags);

    attr->max_wr    = srq->rq.wqe_max;
	attr->max_sge   = srq->rq.sge_max;
	attr->srq_limit = srq->srq_limit;

    spin_unlock_irqrestore(&srq->srq_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_post_srq_recv(struct ib_srq *ibsrq, 
                        const struct ib_recv_wr *recv_wr,
                        const struct ib_recv_wr **bad_wr)
{
    int ret;
    unsigned long flags;
    struct dis_srq *srq = to_dis_srq(ibsrq);
    const struct ib_recv_wr *recv_wr_iter;
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&srq->srq_lock, flags);

    recv_wr_iter = recv_wr;
    while (recv_wr_iter) {
        ret = dis_qp_post_one_rqe(&srq->rq,
                                    recv_wr_iter);
        if (ret) {
            spin_unlock_irqrestore(&srq->srq_lock, flags);
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        dis_qp_notify(&srq->rq);
        recv_wr_iter = recv_wr_iter->next;
    }
    spin_unlock_irqrestore(&srq->srq_lock, flags);
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void dis_destroy_srq(struct ib_srq *ibsrq,  struct ib_udata *udata)
{
    struct dis_srq *srq = to_dis_srq(ibsrq);
    pr_devel(DIS_STATUS_START);
    
    kfree(srq->rq.wqe_queue);

    pr_devel(DIS_STATUS_COMPLETE);
}
