#include "dis_verbs.h"

int dis_query_device(struct ibv_context *uctx, struct ibv_device_attr *attr)
{
    // attr->fw_ver            = 0;

    // attr->node_guid         = 0;
    // attr->sys_image_guid    = 0;
    // attr->max_mr_size       = 0;
    // attr->page_size_cap     = 0;
    // attr->vendor_id         = 0;
    // attr->vendor_part_id    = 0;
    // attr->hw_ver            = 0;
    // attr->max_qp            = 0;
    // attr->max_qp_wr         = 0;
    // attr->device_cap_flags  = 0;
    // attr->max_sge           = 0;
    // attr->max_sge_rd	    = 0;
	// attr->max_cq		    = 0;
	// attr->max_cqe	        = 0;
	// attr->max_mr	    	= 0;
	// attr->max_pd		    = 0;
	// attr->max_qp_rd_atom    = 0;
	// attr->max_ee_rd_atom	= 0;
	// attr->max_res_rd_atom	= 0;
	// attr->max_qp_init_rd_atom	    = 0;
	// attr->max_ee_init_rd_atom	    = 0;
	// attr->atomic_cap		= 0;
	// attr->max_ee			= 0;
	// attr->max_rdd			= 0;
	// attr->max_mw			= 0;
	// attr->max_raw_ipv6_qp	= 0;
	// attr->max_raw_ethy_qp	= 0;
	// attr->max_mcast_grp		= 0;
	// attr->max_mcast_qp_attach       = 0;
	// attr->max_total_mcast_qp_attach	= 0;
	// attr->max_ah			= 0;
	// attr->max_fmr			= 0;
	// attr->max_map_per_fmr	= 0;
	// attr->max_srq			= 0;
	// attr->max_srq_wr		= 0;
	// attr->max_srq_sge		= 0;
	// attr->max_pkeys			= 0;
	// attr->local_ca_ack_delay        = 0;
	// attr->phys_port_cnt		= 0;
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