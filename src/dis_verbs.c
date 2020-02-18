#include "dis_verbs.h"

int dis_query_device(struct ib_device *ibdev, struct ib_device_attr *props,
                     struct ib_udata *udata)
{
    return 0;
}

int dis_query_port(struct ib_device *ibdev, u8 port,
                   struct ib_port_attr *props)
{
    return 0;
}

int dis_get_port_immutable(struct ib_device *ibdev, u8 port_num,
				   struct ib_port_immutable *immutable)
{
    return 0;
}

int dis_query_pkey(struct ib_device *ibdev, u8 port, u16 index,
			       u16 *pkey)
{
    return 0;   
}

int dis_alloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    return 0;
}

void dis_dealloc_pd(struct ib_pd *ibpd, struct ib_udata *udata)
{
    
}

struct ib_mr *dis_get_dma_mr(struct ib_pd *ibpd, int acc)
{
    return 0;
}

int dis_dereg_mr(struct ib_mr *ibmr, struct ib_udata *udata)
{
    return 0;
}

int dis_create_cq(struct ib_cq *ibcq,
                            const struct ib_cq_init_attr *attr,
                            struct ib_udata *udata)
{
    return 0;
}

int dis_poll_cq(struct ib_cq *ibcq, int nwc, struct ib_wc *wc)
{
    return 0;
}

int dis_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
    return 0;
}

void dis_destroy_cq(struct ib_cq *ibcq, struct ib_udata *udata)
{
    
}

struct ib_qp *dis_create_qp(struct ib_pd *ibpd,
                            struct ib_qp_init_attr *init_attr,
                            struct ib_udata *udata)
{
    return 0;
}

int dis_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
                 int qp_attr_mask,
                 struct ib_qp_init_attr *qp_init_attr)
{
    return 0;
}

int dis_modify_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
                  int qp_attr_mask, struct ib_udata *udata)
{
    return 0;
}

int dis_destroy_qp(struct ib_qp *ibqp, struct ib_udata *udata)
{
    return 0;
}

int dis_post_send(struct ib_qp *ibqp, const struct ib_send_wr *send_wr,
                  const struct ib_send_wr **bad_wr)
{
    return 0;
}

int dis_post_recv(struct ib_qp *ibqp, const struct ib_recv_wr *recv_wr,
                  const struct ib_recv_wr **bad_wr)
{
    return 0;
}