#include "pr_fmt.h"

#include "dis_verbs.h"
#include "dis_qp.h"

static int global_qpn = 100;

int dis_query_device(struct ib_device *ibdev, struct ib_device_attr *dev_attr,
                        struct ib_udata *udata)
{
    pr_devel(DIS_STATUS_START);

    dev_attr->fw_ver               = 1;
    dev_attr->sys_image_guid       = 1234;
    dev_attr->max_mr_size          = ~0ull;
    dev_attr->page_size_cap        = 0xffff000; // 4KB-128MB
    dev_attr->vendor_id            = 1234;
    dev_attr->vendor_part_id       = 1234;
    dev_attr->hw_ver               = 1;
    dev_attr->max_qp               = 1234;
    dev_attr->max_qp_wr            = 1234;
    dev_attr->device_cap_flags     = IB_DEVICE_PORT_ACTIVE_EVENT;
    dev_attr->device_cap_flags     |= IB_DEVICE_LOCAL_DMA_LKEY;
    dev_attr->device_cap_flags     |= IB_DEVICE_MEM_MGT_EXTENSIONS;  // Support FR
    dev_attr->max_send_sge         = 1234;
    dev_attr->max_recv_sge         = 1234;
    dev_attr->max_sge_rd           = 1;
    dev_attr->max_cq               = 1234;
    dev_attr->max_cqe              = 1234;
    dev_attr->max_mr               = 1234;
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
    //port_attr->port_cap_flags   = IB_PORT_CM_SUP;
    // port_attr->port_cap_flags   = IB_PORT_REINIT_SUP;
    // port_attr->port_cap_flags   |= IB_PORT_DEVICE_MGMT_SUP;
    // port_attr->port_cap_flags   |= IB_PORT_VENDOR_CLASS_SUP;
    port_attr->gid_tbl_len      = 1;
    port_attr->pkey_tbl_len     = 1;
	port_attr->max_vl_num       = 1;
    port_attr->max_msg_sz       = 0x80000000;
    port_attr->max_mtu          = 4096;
	port_attr->active_mtu       = 4096;
	port_attr->lid              = 0;
	port_attr->sm_lid           = 0;
	port_attr->bad_pkey_cntr    = 0;
	port_attr->qkey_viol_cntr   = 0;
	port_attr->lmc              = 0;
	port_attr->sm_sl            = 0;
	port_attr->subnet_timeout   = 0;
	port_attr->init_type_reply  = 0;

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
    struct dis_pd *dispd    = to_dis_pd(ibpd);
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(DIS_STATUS_START);

    dispd->disdev = disdev;

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
    struct dis_mr *dismr;
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(DIS_STATUS_START);

    dismr = kzalloc(sizeof(struct dis_mr), GFP_KERNEL);
    if (!dismr) {
        dev_err(&ibdev->dev, "dis_get_dma_mr " DIS_STATUS_FAIL);
        return ERR_PTR(-1);
    }

    dismr->disdev = disdev;

    pr_devel(DIS_STATUS_COMPLETE);
    return &dismr->ibmr;
}

int dis_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
{
    struct dis_mr *dismr = to_dis_mr(ibmr);

    pr_devel(DIS_STATUS_START);

    //TODO: Reuse memory?
    kfree(dismr);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_create_cq(struct ib_cq *ibcq, const struct ib_cq_init_attr *init_attr,
                    struct ib_udata *udata)
{
    // int ret;
    struct dis_cq *discq    = to_dis_cq(ibcq);
    struct ib_device *ibdev = ibcq->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(DIS_STATUS_START);
    
    //TODO: Ensure CQE count does not exceed max.
    //discq->max_cqe  = init_attr->cqe;
    ibcq->cqe       = init_attr->cqe;
    discq->disdev   = disdev;

    // discq->queue.max_elem   = init_attr->cqe;
    // discq->queue.elem_size  = sizeof(struct dis_cqe);
    // ret = dis_create_queue(&discq->queue);
    // if (ret) {
    //     dev_err(&ibdev->dev, "Create queue: " DIS_STATUS_FAIL);
    //     return -42;
    // }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_poll_cq(struct ib_cq *ibcq, int num_wc, struct ib_wc *ibwc)
{
    // struct dis_cq *discq = to_dis_cq(ibcq);
    // //struct dis_cqe* discqe;
    // unsigned long flags;
    // int i;

    pr_devel(DIS_STATUS_START);
    

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
    pr_devel(DIS_STATUS_START);

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

void dis_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
{
    // struct dis_cq *discq = to_dis_cq(ibcq);

    pr_devel(DIS_STATUS_START);

    // dis_destroy_queue(&discq->queue);

    pr_devel(DIS_STATUS_COMPLETE);
}

struct ib_qp *dis_create_qp(struct ib_pd *ibpd,
                            struct ib_qp_init_attr *init_attr,
                            struct ib_udata *udata)
{
    struct dis_qp *disqp;
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);
    pr_devel(DIS_STATUS_START);

    disqp = kzalloc(sizeof(struct dis_qp), GFP_KERNEL);
    if (!disqp) {
        goto create_qp_alloc_qp_err;
    }

    disqp->disdev           = disdev;
    disqp->sq_sig_type      = init_attr->sq_sig_type;
    disqp->type             = init_attr->qp_type;
    disqp->state            = IB_QPS_RESET;
    disqp->mtu              = ib_mtu_enum_to_int(IB_MTU_256);
    disqp->l_qpn            = global_qpn++;
    disqp->event_handler    = init_attr->event_handler;

    disqp->ibqp.pd          = ibpd;
    disqp->ibqp.send_cq     = init_attr->send_cq;
    disqp->ibqp.recv_cq     = init_attr->recv_cq;
    disqp->ibqp.srq         = init_attr->srq;
    disqp->ibqp.qp_type     = init_attr->qp_type;
    disqp->ibqp.qp_num      = disqp->l_qpn;

    disqp->sq.discq         = to_dis_cq(init_attr->send_cq);
    disqp->sq.max_wqe       = roundup_pow_of_two(init_attr->cap.max_send_wr);
    disqp->sq.max_sge       = init_attr->cap.max_send_sge;
    disqp->sq.max_inline    = init_attr->cap.max_inline_data;

    disqp->sq.wqe = kzalloc(sizeof(struct dis_wqe) * disqp->sq.max_wqe, 
                            GFP_KERNEL);
    if (!disqp->sq.wqe) {
        pr_devel(DIS_STATUS_FAIL);
        goto create_qp_alloc_sq_err;
    }
    memset(disqp->sq.wqe, 0, sizeof(struct dis_wqe) * disqp->sq.max_wqe);

    disqp->rq.discq         = to_dis_cq(init_attr->recv_cq);
    disqp->rq.max_wqe       = roundup_pow_of_two(init_attr->cap.max_recv_wr);
    disqp->rq.max_sge       = init_attr->cap.max_recv_sge;
    disqp->rq.max_inline    = init_attr->cap.max_inline_data;

    disqp->rq.wqe = kzalloc(sizeof(struct dis_wqe) * disqp->rq.max_wqe, 
                            GFP_KERNEL);
    if (!disqp->rq.wqe) {
        pr_devel(DIS_STATUS_FAIL);
        goto create_qp_alloc_rq_err;
    }
    memset(disqp->rq.wqe, 0, sizeof(struct dis_wqe) * disqp->rq.max_wqe);
    
    return &disqp->ibqp;

create_qp_alloc_rq_err:
    kfree(disqp->sq.wqe);

create_qp_alloc_sq_err:
    kfree(disqp);

create_qp_alloc_qp_err:
    return ERR_PTR(-1);
}

int dis_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
                    int attr_mask,
                    struct ib_qp_init_attr *init_attr)
{
    pr_devel(DIS_STATUS_START);

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
                    int attr_mask, struct ib_udata *udata)
{
    int ret;
    struct dis_qp *disqp = to_dis_qp(ibqp);
    pr_devel(DIS_STATUS_START);

    if (attr_mask & IB_QP_STATE) {
		switch (attr->qp_state) {
		case IB_QPS_RESET:
			pr_devel("Modify QP state: RESET");
			break;

		case IB_QPS_INIT:
			pr_devel("Modify QP state: INIT");
            
			break;

		case IB_QPS_RTR:
			pr_devel("Modify QP state: RTR");

            if (attr_mask & IB_QP_DEST_QPN) {
                // disqp->r_qpn    = attr->dest_qp_num;
                disqp->r_qpn    = disqp->l_qpn;

                disqp->rq.l_qpn = disqp->l_qpn;
                disqp->rq.r_qpn = disqp->r_qpn;

                disqp->sq.l_qpn = disqp->l_qpn;
                disqp->sq.r_qpn = disqp->r_qpn;
            }
            
            disqp->rq.wq_type = DIS_RQ;
            ret = dis_qp_init(&disqp->rq);
            if(ret) {
                goto dis_rq_init_err;
            }
			break;

		case IB_QPS_RTS:
			pr_devel("Modify QP state: RTS");

            disqp->sq.wq_type = DIS_SQ;
            ret = dis_qp_init(&disqp->sq);
            if(ret) {
                goto dis_sq_init_err;
            }
			break;

		default:
			pr_devel(DIS_STATUS_FAIL);
			return -42;
		}

        disqp->state = attr->qp_state;
	}

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;

dis_sq_init_err:
dis_rq_init_err:
    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

int dis_destroy_qp(struct ib_qp *ibqp, struct ib_udata *udata)
{
    struct dis_qp *disqp = to_dis_qp(ibqp);
    // struct ib_device *ibdev = ibqp->device;
    // struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(DIS_STATUS_START);

    // dis_qp_exit(disqp);
    dis_qp_exit(&disqp->rq);
    dis_qp_exit(&disqp->sq);
    kfree(disqp->rq.wqe);
    kfree(disqp->sq.wqe);
    kfree(disqp);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_post_send(struct ib_qp *ibqp, const struct ib_send_wr *send_wr,
                    const struct ib_send_wr **bad_wr)
{
    int i;
    struct dis_qp *disqp = to_dis_qp(ibqp);
    struct dis_wqe *sqe;
    const struct ib_send_wr *send_wr_iter;
    pr_devel(DIS_STATUS_START);

    send_wr_iter = send_wr;
    while (send_wr_iter) {
        sqe = &disqp->sq.wqe[disqp->sq.wqe_put % disqp->sq.max_wqe];

        if(sqe->flags != DIS_WQE_FREE) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        for (i = 0; i < min(send_wr_iter->num_sge, DIS_MAX_SGE); i++) {
            sqe->sg_list[i].addr   = send_wr_iter->sg_list[i].addr;
            sqe->sg_list[i].length = send_wr_iter->sg_list[i].length;
            sqe->sg_list[i].lkey   = send_wr_iter->sg_list[i].lkey;
            sqe->num_sge++;
        }

        sqe->flags = DIS_WQE_VALID;
        disqp->sq.wqe_put++;
        dis_qp_post(&disqp->sq);
        send_wr_iter = send_wr_iter->next;
    }
    
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int dis_post_recv(struct ib_qp *ibqp, const struct ib_recv_wr *recv_wr,
                    const struct ib_recv_wr **bad_wr)
{
    pr_devel(DIS_STATUS_START);

    // disqp->rq.wq_flag = SCI_IF_POST_RECV;
    // dis_rq_signal(disqp);
    
    pr_devel(DIS_STATUS_FAIL);
    return -42;
}