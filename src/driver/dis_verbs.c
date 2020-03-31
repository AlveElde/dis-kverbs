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
    dev_attr->page_size_cap        = 10000;
    dev_attr->vendor_id            = 0x1234;
    dev_attr->vendor_part_id       = 0x1234;
    dev_attr->hw_ver               = 1234;
    dev_attr->max_qp               = 1234;
    dev_attr->max_qp_wr            = 1234;
    dev_attr->device_cap_flags     = IB_DEVICE_ALLOW_USER_UNREG;
    dev_attr->device_cap_flags     |= IB_DEVICE_MEM_MGT_EXTENSIONS;
    dev_attr->max_send_sge         = DIS_WQE_SGE_MAX;
    dev_attr->max_recv_sge         = DIS_WQE_SGE_MAX;
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
    port_attr->max_msg_sz       = 4096;
    port_attr->max_mtu          = IB_MTU_4096;
    port_attr->active_mtu       = IB_MTU_4096;
    port_attr->lid              = 0;
    port_attr->sm_lid           = 0;
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
                                u64 hca_va,
                                int access,
                                struct ib_udata *udata)
{
    int i;
    struct dis_mr *mr;
    struct dis_pd *pd = to_dis_pd(ibpd);
    struct dis_map **map;
    struct dis_pb *pb;
    struct sg_page_iter	sg_iter;
    pr_devel(DIS_STATUS_START);

    if (pd->mr_c >= DIS_MR_MAX) {
        pr_devel(DIS_STATUS_FAIL);
        goto no_mr_left_err;
    }

    mr = &pd->mr_list[pd->mr_c];

    mr->ibumem  = ib_umem_get(udata, start, length, access);
    if (IS_ERR(mr->ibumem)) {
        pr_devel(DIS_STATUS_FAIL);
        goto ib_umem_get_err;
    }

    mr->page_count = ib_umem_page_count(mr->ibumem); //ib_umem_num_pages ?
    if (!mr->page_count) {
        pr_devel(DIS_STATUS_FAIL);
        goto page_count_err;
    }

    mr->map_count   = (mr->page_count + DIS_PB_PER_MAP - 1) / DIS_PB_PER_MAP;
    mr->map_list    = kmalloc_array(mr->map_count, 
                                    sizeof(struct dis_map *),
                                    GFP_KERNEL);
    if (!mr->map_list) {
        pr_devel(DIS_STATUS_FAIL);
        goto alloc_map_list_array_err;
    }

    for (i = 0; i < mr->map_count; i++) {
        mr->map_list[i] = kmalloc(sizeof(struct dis_map), GFP_KERNEL);

        if (!mr->map_list[i]) {
            pr_devel(DIS_STATUS_FAIL);
            for (i--; i >= 0; i--) {
                kfree(mr->map_list[i]);
            }
            goto alloc_map_list_err;
        }
    }

    map = mr->map_list;
    pb = (*map)->pb_list;
    i = 0;
    for_each_sg_page (mr->ibumem->sg_head.sgl, &sg_iter, mr->ibumem->nmap, 0) {
        if(i == DIS_PB_PER_MAP) {
            map++;
            pb = (*map)->pb_list;
            i = 0;
        }

        pb->phys_addr = (uintptr_t)page_address(sg_page_iter_page(&sg_iter));
        pb->length = PAGE_SIZE;
        if (!pb->phys_addr) {
            pr_devel(DIS_STATUS_FAIL);
            goto phys_addr_err;
        }

        pb++;
        i++;
    }

    mr->offset = ib_umem_offset(mr->ibumem);
    mr->virt_addr = hca_va;
    mr->ibmr.lkey = pd->mr_c++;

    pr_devel(DIS_STATUS_COMPLETE);
    return &mr->ibmr;

phys_addr_err:
    for (i = 0; i < mr->map_count; i++) {
        kfree(mr->map_list[i]);
    }

alloc_map_list_err:
    kfree(mr->map_list);

alloc_map_list_array_err:
page_count_err:
    ib_umem_release(mr->ibumem);

ib_umem_get_err:
no_mr_left_err:
    return ERR_PTR(-EINVAL);
}

int dis_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
{
    int i;
    struct dis_mr *mr = to_dis_mr(ibmr);
    pr_devel(DIS_STATUS_START);

    for (i = 0; i < mr->map_count; i++) {
        kfree(mr->map_list[i]);
    }

    kfree(mr->map_list);

    if (mr->ibumem) {
        ib_umem_release(mr->ibumem);
    }

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
    int wc_count;
    unsigned long flags;
    struct dis_cq *cq = to_dis_cq(ibcq);
    struct dis_cqe* cqe;
    struct ib_wc *ibwc_iter;
    pr_devel(DIS_STATUS_START);

    ibwc_iter = ibwc;
    for (wc_count = 0; wc_count < num_wc; wc_count++) {
        spin_lock_irqsave(&cq->cqe_lock, flags);

        cqe = cq->cqe_queue + (cq->cqe_get % cq->cqe_max);

        if(!cqe->valid) {
            spin_unlock_irqrestore(&cq->cqe_lock, flags);
            break;
        }
        ibwc_iter->wr_id    = cqe->id;
        ibwc_iter->status   = cqe->status;
        ibwc_iter->opcode   = cqe->opcode;
        ibwc_iter->byte_len = cqe->byte_len;
        ibwc_iter->qp       = cqe->ibqp;

        cqe->valid = 0;
        cq->cqe_get++;
        spin_unlock_irqrestore(&cq->cqe_lock, flags);
        ibwc_iter++;
    }
    
    pr_devel(DIS_STATUS_COMPLETE);
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
        goto create_qp_alloc_qp_err;
    }

    qp = &pd->qp_list[pd->qp_c];

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

    qp->sq.cq           = to_dis_cq(init_attr->send_cq);
    qp->sq.sge_max      = init_attr->cap.max_send_sge;
    qp->sq.inline_max   = init_attr->cap.max_inline_data;
    qp->sq.wqe_get      = 0;
    qp->sq.wqe_put      = 0;
    qp->sq.wq_type      = DIS_SQ;
    qp->sq.wqe_max      = roundup_pow_of_two(init_attr->cap.max_send_wr);
    qp->sq.wqe_queue    = kzalloc(sizeof(struct dis_wqe) * qp->sq.wqe_max, 
                            GFP_KERNEL);
    if (!qp->sq.wqe_queue) {
        pr_devel(DIS_STATUS_FAIL);
        goto create_qp_alloc_sq_err;
    }
    memset(qp->sq.wqe_queue, 0, sizeof(struct dis_wqe) * qp->sq.wqe_max);

    qp->rq.cq           = to_dis_cq(init_attr->recv_cq);
    qp->rq.sge_max      = init_attr->cap.max_recv_sge;
    qp->rq.inline_max   = init_attr->cap.max_inline_data;
    qp->rq.wqe_get      = 0;
    qp->rq.wqe_put      = 0;
    qp->rq.wq_type      = DIS_RQ;
    qp->rq.wqe_max      = roundup_pow_of_two(init_attr->cap.max_recv_wr);
    qp->rq.wqe_queue    = kzalloc(sizeof(struct dis_wqe) * qp->rq.wqe_max, 
                            GFP_KERNEL);
    if (!qp->rq.wqe_queue) {
        pr_devel(DIS_STATUS_FAIL);
        goto create_qp_alloc_rq_err;
    }
    memset(qp->rq.wqe_queue, 0, sizeof(struct dis_wqe) * qp->rq.wqe_max);

    pd->qp_c++;
    
    pr_devel(DIS_STATUS_COMPLETE);
    return &qp->ibqp;

create_qp_alloc_rq_err:
    kfree(qp->sq.wqe_queue);

create_qp_alloc_sq_err:
    kfree(qp);

create_qp_alloc_qp_err:
    return NULL;
}

int dis_query_qp(struct ib_qp *ibqp, 
                    struct ib_qp_attr *attr,
                    int attr_mask,
                    struct ib_qp_init_attr *init_attr)
{
    pr_devel(DIS_STATUS_START);

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
            pr_devel("Modify QP state: RESET");
            break;

        case IB_QPS_INIT:
            pr_devel("Modify QP state: INIT");
            qp->rq.mtu = qp->mtu;
            qp->sq.mtu = qp->mtu;
            break;

        case IB_QPS_RTR:
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

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_post_send(struct ib_qp *ibqp, 
                    const struct ib_send_wr *send_wr,
                    const struct ib_send_wr **bad_wr)
{
    int i, pb_i;
    u64 sge_virt_addr, offset;
    struct dis_qp *qp = to_dis_qp(ibqp);
    struct dis_pd *pd = to_dis_pd(ibqp->pd);
    struct dis_wqe *sqe;
    struct dis_mr *mr;
    struct dis_map **map;
    struct dis_pb *pb;
    const struct ib_send_wr *send_wr_iter;
    pr_devel(DIS_STATUS_START);

    send_wr_iter = send_wr;
    while (send_wr_iter) {
        sqe = qp->sq.wqe_queue + (qp->sq.wqe_put % qp->sq.wqe_max);

        if(sqe->valid) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        sqe->valid      = 1;
        sqe->opcode     = IB_WC_SEND;
        sqe->sci_msq    = &qp->sq.sci_msq;
        sqe->byte_len   = 0;
        sqe->ibqp       = ibqp;

        sqe->sci_msg.cmsg_valid = 0;
        sqe->sci_msg.page       = NULL;
        sqe->sci_msg.iov        = sqe->iov;
        sqe->sci_msg.iovlen     = min(send_wr_iter->num_sge, DIS_WQE_SGE_MAX);

        for (i = 0; i < sqe->sci_msg.iovlen; i++) {
            mr      = &pd->mr_list[send_wr_iter->sg_list[i].lkey];
            map     = mr->map_list;
            pb      = (*map)->pb_list;
            pb_i    = 0;
            sge_virt_addr = (uintptr_t)(send_wr_iter->sg_list[i].addr);
            offset  = (sge_virt_addr - mr->virt_addr) + mr->offset;
            
            while (offset >= pb->length) {
                offset -= pb->length;
                pb++;
                pb_i++;
                if (pb_i == DIS_PB_PER_MAP) {
                    map++;
                    pb = (*map)->pb_list;
                    pb_i = 0;
                }
            }

            sqe->iov[i].iov_base    = (void *)(pb->phys_addr + offset);
            sqe->iov[i].iov_len     = (size_t)(send_wr_iter->sg_list[i].length);
            sqe->byte_len           += send_wr_iter->sg_list[i].length;
            sqe->lkey[i]            = send_wr_iter->sg_list[i].lkey;
        }

        qp->sq.wqe_put++;
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
    int i, pb_i;
    u64 sge_virt_addr, offset;
    struct dis_qp *qp = to_dis_qp(ibqp);
    struct dis_pd *pd = to_dis_pd(ibqp->pd);
    struct dis_wqe *rqe;
    struct dis_mr *mr;
    struct dis_map **map;
    struct dis_pb *pb;
    const struct ib_recv_wr *recv_wr_iter;
    pr_devel(DIS_STATUS_START);

    recv_wr_iter = recv_wr;
    while (recv_wr_iter) {
        rqe = qp->rq.wqe_queue + (qp->rq.wqe_put % qp->rq.wqe_max);

        if(rqe->valid) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        rqe->valid      = 1;
        rqe->opcode     = IB_WC_RECV;
        rqe->sci_msq    = &qp->rq.sci_msq;
        rqe->byte_len   = 0;
        rqe->ibqp       = ibqp;

        rqe->sci_msg.cmsg_valid = 0;
        rqe->sci_msg.page       = NULL;
        rqe->sci_msg.iov        = rqe->iov;
        rqe->sci_msg.iovlen     = min(recv_wr_iter->num_sge, DIS_WQE_SGE_MAX);

        for (i = 0; i < rqe->sci_msg.iovlen; i++) {
            mr      = &pd->mr_list[recv_wr_iter->sg_list[i].lkey];
            map     = mr->map_list;
            pb      = (*map)->pb_list;
            pb_i    = 0;
            sge_virt_addr = (uintptr_t)(recv_wr_iter->sg_list[i].addr);
            offset  = (sge_virt_addr - mr->virt_addr) + mr->offset;
            
            while (offset >= pb->length) {
                offset -= pb->length;
                pb++;
                pb_i++;
                if (pb_i == DIS_PB_PER_MAP) {
                    map++;
                    pb = (*map)->pb_list;
                    pb_i = 0;
                }
            }


            rqe->iov[i].iov_base    = (void *)(pb->phys_addr + offset);
            rqe->iov[i].iov_len     = (size_t)(recv_wr_iter->sg_list[i].length);
            rqe->byte_len           += recv_wr_iter->sg_list[i].length;
            rqe->lkey[i]            = recv_wr_iter->sg_list[i].lkey;
        }

        qp->rq.wqe_put++;
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
    
    //TODO

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
    int i;
    unsigned long flags;
    struct dis_srq *srq = to_dis_srq(ibsrq);
    struct dis_wqe *rqe;
    const struct ib_recv_wr *recv_wr_iter;
    pr_devel(DIS_STATUS_START);
    spin_lock_irqsave(&srq->srq_lock, flags);

    recv_wr_iter = recv_wr;
    while (recv_wr_iter) {
        rqe = srq->rq.wqe_queue + (srq->rq.wqe_put % srq->rq.wqe_max);

        if(rqe->valid) {
            pr_devel(DIS_STATUS_FAIL);
            spin_unlock_irqrestore(&srq->srq_lock, flags);
            return -42;
        }

        rqe->valid      = 1;
        rqe->opcode     = IB_WC_RECV;
        // rqe->sci_msq    = &qp->rq.sci_msq;

        rqe->sci_msg.cmsg_valid = 0;
        rqe->sci_msg.page       = NULL;
        rqe->sci_msg.iov        = rqe->iov;
        rqe->sci_msg.iovlen     = min(recv_wr_iter->num_sge, DIS_WQE_SGE_MAX);

        rqe->byte_len = 0;
        for (i = 0; i < rqe->sci_msg.iovlen; i++) {
            rqe->iov[i].iov_base    = (void *)(recv_wr_iter->sg_list[i].addr);
            rqe->iov[i].iov_len     = (size_t)(recv_wr_iter->sg_list[i].length);
            rqe->byte_len           += recv_wr_iter->sg_list[i].length;
            rqe->lkey[i]            = recv_wr_iter->sg_list[i].lkey;
        }

        srq->rq.wqe_put++;
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
