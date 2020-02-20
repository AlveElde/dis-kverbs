#include "dis_verbs.h"

int dis_query_device(struct ib_device *ibdev, struct ib_device_attr *props,
                        struct ib_udata *udata)
{
	props->fw_ver               = 1;
	props->sys_image_guid       = 1234;
	props->max_mr_size          = ~0ull;
	props->page_size_cap        = 0xffff000  /* 4KB-128MB */;
	props->vendor_id            = 1234;
	props->vendor_part_id       = 1234;
	props->hw_ver               = 1;
	props->max_qp               = 1234;
	props->max_qp_wr            = 1234;
	props->device_cap_flags     = IB_DEVICE_PORT_ACTIVE_EVENT;
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

    return 0;
}

int dis_query_port(struct ib_device *ibdev, u8 port,
                    struct ib_port_attr *props)
{
    //props->port_cap_flags   = IB_PORT_CM_SUP;
    // props->port_cap_flags   = IB_PORT_REINIT_SUP;
    // props->port_cap_flags   |= IB_PORT_DEVICE_MGMT_SUP;
    // props->port_cap_flags   |= IB_PORT_VENDOR_CLASS_SUP;
	props->gid_tbl_len      = 1;
	props->pkey_tbl_len     = 1;
	props->max_msg_sz       = 0x80000000;

    return 0;
}

int dis_get_port_immutable(struct ib_device *ibdev, u8 port_num,
				            struct ib_port_immutable *immutable)
{
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
    return 0;
}

int dis_query_pkey(struct ib_device *ibdev, u8 port, u16 index,
			        u16 *pkey)
{
    *pkey = 0xffff;
    
    return 0;   
}

int dis_alloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    return -42;
}

void dis_dealloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    
}

struct ib_mr *dis_get_dma_mr(struct ib_pd *ibpd, int acc)
{
    struct dis_mr *dismr;

    dismr = kzalloc(sizeof(*dismr), GFP_KERNEL);
	if (!dismr)
		return ERR_PTR(-ENOMEM);

    return NULL;
}

int dis_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
{
    return -42;
}

int dis_create_cq(struct ib_cq *ibcq,
                    const struct ib_cq_init_attr *attr,
                    struct ib_udata *udata)
{
    return -42;
}

int dis_poll_cq(struct ib_cq *ibcq, int nwc, struct ib_wc *wc)
{
    return -42;
}

int dis_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
    return -42;
}

void dis_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
{
    
}

struct ib_qp *dis_create_qp(struct ib_pd *ibpd,
                            struct ib_qp_init_attr *init_attr,
                            struct ib_udata *udata)
{
    return NULL;
}

int dis_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
                    int qp_attr_mask,
                    struct ib_qp_init_attr *qp_init_attr)
{
    return -42;
}

int dis_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
                    int qp_attr_mask, struct ib_udata *udata)
{
    return -42;
}

int dis_destroy_qp(struct ib_qp *ibqp, struct ib_udata *udata)
{
    return -42;
}

int dis_post_send(struct ib_qp *ibqp, const struct ib_send_wr *send_wr,
                    const struct ib_send_wr **bad_wr)
{
    return -42;
}

int dis_post_recv(struct ib_qp *ibqp, const struct ib_recv_wr *recv_wr,
                    const struct ib_recv_wr **bad_wr)
{
    return -42;
}