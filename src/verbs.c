#include "verbs.h"

int dis_query_device(struct ibv_context *uctx, struct ibv_device_attr *attr)
{

}

int dis_query_port(struct ibv_context *uctx, uint8_t port, struct ibv_port_attr *attr)
{

}

struct ibv_pd *dis_alloc_pd(struct ibv_context *uctx)
{

}

int dis_dealloc_pd(struct ibv_pd *ibvpd)
{

}

struct ibv_mr *dis_reg_mr(struct ibv_pd *ibvpd, void *buf, size_t len, uint64_t hca_va, int ibv_access_flags)
{

}

int dis_dereg_mr(struct verbs_mr *vmr)
{

}

struct ibv_cq *dis_create_cq(struct ibv_context *uctx, int ncqe, struct ibv_comp_channel *ch, int vec)
{

}

int dis_poll_cq(struct ibv_cq *ibvcq, int nwc, struct ibv_wc *wc)
{

}

int dis_destroy_cq(struct ibv_cq *ibvcq)
{

}

struct ibv_qp *dis_create_qp(struct ibv_pd *ibvpd, struct ibv_qp_init_attr *attr)
{

}

int dis_query_qp(struct ibv_qp *ibvqp, struct ibv_qp_attr *attr, int attr_mask, struct ibv_qp_init_attr *init_attr)
{

}

int dis_modify_qp(struct ibv_qp *ibvqp, struct ibv_qp_attr *attr, int ibv_qp_attr_mask)
{

}

int dis_destroy_qp(struct ibv_qp *ibvqp)
{

}

int dis_post_send(struct ibv_qp *ibvqp, struct ibv_send_wr *wr, struct ibv_send_wr **bad)
{

}

int dis_post_recv(struct ibv_qp *ibvqp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad)
{

}

struct ibv_ah *dis_create_ah(struct ibv_pd *ibvpd, struct ibv_ah_attr *attr)
{

}

int dis_destroy_ah(struct ibv_ah *ibvah)
{

}