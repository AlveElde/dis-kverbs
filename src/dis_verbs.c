#define DEBUG
#define pr_fmt(fmt) KBUILD_MODNAME ": fn: %s, ln: %d: " fmt, __func__, __LINE__

#include "dis_verbs.h"
#include "dis_driver.h"

int dis_query_device(struct ib_device *ibdev, struct ib_device_attr *props,
                        struct ib_udata *udata)
{
    pr_devel(STATUS_START);

	props->fw_ver               = 1;
	props->sys_image_guid       = 1234;
	props->max_mr_size          = ~0ull;
	props->page_size_cap        = 0xffff000; // 4KB-128MB
	props->vendor_id            = 1234;
	props->vendor_part_id       = 1234;
	props->hw_ver               = 1;
	props->max_qp               = 1234;
	props->max_qp_wr            = 1234;
	props->device_cap_flags     = IB_DEVICE_PORT_ACTIVE_EVENT;
    props->device_cap_flags     |= IB_DEVICE_LOCAL_DMA_LKEY; 
    props->device_cap_flags     |= IB_DEVICE_MEM_MGT_EXTENSIONS; // Support FR
	props->max_send_sge         = 1234;
	props->max_recv_sge         = 1234;
	props->max_sge_rd           = 1;
	props->max_cq               = 1234;
	props->max_cqe              = 1234;
	props->max_mr               = 1234;
	props->max_pd               = 1234;
	props->max_qp_rd_atom       = 0;
	props->max_qp_init_rd_atom  = 0;
	props->atomic_cap           = IB_ATOMIC_NONE;
	props->max_pkeys            = 1;
	props->local_ca_ack_delay   = 1;

    pr_devel(STATUS_COMPLETE);
    return 0;
}

int dis_query_port(struct ib_device *ibdev, u8 port,
                    struct ib_port_attr *props)
{
    pr_devel(STATUS_START);
    //props->port_cap_flags   = IB_PORT_CM_SUP;
    // props->port_cap_flags   = IB_PORT_REINIT_SUP;
    // props->port_cap_flags   |= IB_PORT_DEVICE_MGMT_SUP;
    // props->port_cap_flags   |= IB_PORT_VENDOR_CLASS_SUP;
	props->gid_tbl_len      = 1;
	props->pkey_tbl_len     = 1;
	props->max_msg_sz       = 0x80000000;

    pr_devel(STATUS_COMPLETE);
    return 0;
}

int dis_get_port_immutable(struct ib_device *ibdev, u8 port_num,
				            struct ib_port_immutable *immutable)
{
    pr_devel(STATUS_START);
    //int ret;
    // struct ib_port_attr port_attr;
    // ret = dis_query_port(ibdev, port_num, &port_attr);
    // if(ret) {
    //     return -42;
    // }
    
    // TODO: Replace hard-coded values with result from dis_query_port
    immutable->pkey_tbl_len = 1;
	immutable->gid_tbl_len = 1;
	immutable->core_cap_flags = RDMA_CORE_PORT_RAW_PACKET;
    
    // This has to be 0 in order to not trigger the verify_immutable check
	immutable->max_mad_size = 0; //IB_MGMT_MAD_SIZE;
    pr_devel(STATUS_COMPLETE);
    return 0;
}

int dis_query_pkey(struct ib_device *ibdev, u8 port, u16 index,
			        u16 *pkey)
{
    pr_devel(STATUS_START);

    *pkey = 0xffff;
    
    pr_devel(STATUS_COMPLETE);
    return 0;   
}

int dis_alloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    struct dis_pd *dispd    = to_dis_pd(ibpd);
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(STATUS_START);

    dispd->disdev = disdev;

    pr_devel(STATUS_COMPLETE);
    return 0;
}

void dis_dealloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    pr_devel(STATUS_START);
    // Nothing to do.
    pr_devel(STATUS_COMPLETE);
}

struct ib_mr *dis_get_dma_mr(struct ib_pd *ibpd, int access)
{
    struct dis_mr *dismr;
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(STATUS_START);

    dismr = kzalloc(sizeof(struct dis_mr), GFP_KERNEL);
	if (!dismr) {
        dev_err(&ibdev->dev, "dis_get_dma_mr " STATUS_FAIL);
		return NULL;
    }

    dismr->disdev = disdev;

    pr_devel(STATUS_COMPLETE);
    return &dismr->ibmr;
}

int dis_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
{
    struct dis_mr *dismr = to_dis_mr(ibmr);

    pr_devel(STATUS_START);

    //TODO: Reuse memory?
    kfree(dismr);

    pr_devel(STATUS_COMPLETE);
    return 0;
}

int dis_create_cq(struct ib_cq *ibcq, const struct ib_cq_init_attr *init_attr,
                    struct ib_udata *udata)
{
    int ret;
    struct dis_cq *discq    = to_dis_cq(ibcq);
    struct ib_device *ibdev = ibcq->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(STATUS_START);
    
    //TODO: Ensure CQE count does not exceed max.
    //discq->max_cqe  = init_attr->cqe;
    ibcq->cqe       = init_attr->cqe;
    discq->disdev   = disdev;

    discq->queue.max_elem = init_attr->cqe;
    discq->queue.elem_size = sizeof(struct dis_cqe);
    ret = dis_create_queue(&discq->queue);
    if (ret) {
        dev_err(&ibdev->dev, "Create queue: " STATUS_FAIL);
		return -42;
    }

    pr_devel(STATUS_COMPLETE);
    return 0;
}

int dis_poll_cq(struct ib_cq *ibcq, int num_wc, struct ib_wc *ibwc)
{
    // struct dis_cq *discq = to_dis_cq(ibcq);
    // //struct dis_cqe* discqe;
    // unsigned long flags;
    // int i;

    pr_devel(STATUS_START);
    

    pr_devel(STATUS_FAIL);
    return -42;
}

int dis_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
    pr_devel(STATUS_START);

    pr_devel(STATUS_FAIL);
    return -42;
}

void dis_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
{
    struct dis_cq *discq = to_dis_cq(ibcq);

    pr_devel(STATUS_START);

    dis_destroy_queue(&discq->queue);

    pr_devel(STATUS_COMPLETE);
}

struct ib_qp *dis_create_qp(struct ib_pd *ibpd,
                            struct ib_qp_init_attr *init_attr,
                            struct ib_udata *udata)
{
    int ret;
    struct dis_qp *disqp;
    struct ib_device *ibdev = ibpd->device;
    struct dis_dev *disdev  = to_dis_dev(ibdev);

    pr_devel(STATUS_START);

    disqp = kzalloc(sizeof(struct dis_qp), GFP_KERNEL);
	if (!disqp) {
        dev_err(&ibdev->dev, "kzalloc disqp: " STATUS_FAIL);
		return NULL;
    }
    
    disqp->disdev = disdev;

    disqp->sq.ibcq      = init_attr->send_cq;
    disqp->sq.max_wqe   = init_attr->cap.max_send_wr;
    disqp->sq.max_sge   = init_attr->cap.max_send_sge;

    disqp->sq.queue.max_elem    = disqp->sq.max_wqe;
    disqp->sq.queue.elem_size   = sizeof(struct dis_wqe);
    ret = dis_create_queue(&disqp->sq.queue);
    if (!ret) {
        dev_err(&ibdev->dev, "Create Send Queue: " STATUS_FAIL);
		goto sq_err;
    
    }

    disqp->rq.ibcq      = init_attr->recv_cq;
    disqp->rq.max_wqe   = init_attr->cap.max_recv_wr;
    disqp->rq.max_sge   = init_attr->cap.max_recv_sge;

    disqp->rq.queue.max_elem     = disqp->rq.max_wqe;
    disqp->rq.queue.elem_size    = sizeof(struct dis_wqe);
    ret = dis_create_queue(&disqp->rq.queue);
    if (!ret) {
        dev_err(&ibdev->dev, "Create Receive Queue: " STATUS_FAIL);
		goto rq_err;
    
    }
    
    return &disqp->ibqp;
    
    // pr_devel("Destroy RQ");
    // dis_destroy_queue(&disqp->rq.queue);

rq_err:
    pr_devel("Destroy SQ");
    dis_destroy_queue(&disqp->sq.queue);

sq_err:
    pr_devel("Free QP");
    kfree(disqp);

    pr_devel(STATUS_FAIL);
    return NULL;
}

int dis_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
                    int attr_mask,
                    struct ib_qp_init_attr *init_attr)
{
    pr_devel(STATUS_START);

    pr_devel(STATUS_FAIL);
    return -42;
}

int dis_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *attr,
                    int attr_mask, struct ib_udata *udata)
{
    pr_devel(STATUS_START);

    pr_devel(STATUS_FAIL);
    return -42;
}

int dis_destroy_qp(struct ib_qp *ibqp, struct ib_udata *udata)
{
    pr_devel(STATUS_START);

    pr_devel(STATUS_FAIL);
    return -42;
}

int dis_post_send(struct ib_qp *ibqp, const struct ib_send_wr *send_wr,
                    const struct ib_send_wr **bad_wr)
{
    pr_devel(STATUS_START);

    pr_devel(STATUS_FAIL);
    return -42;
}

int dis_post_recv(struct ib_qp *ibqp, const struct ib_recv_wr *recv_wr,
                    const struct ib_recv_wr **bad_wr)
{
    pr_devel(STATUS_START);

    pr_devel(STATUS_FAIL);
    return -42;
}