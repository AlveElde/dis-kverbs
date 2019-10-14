#include "dis.h"
#include "verbs.h"

#ifndef PCI_VENDOR_ID_DIS
#define PCI_VENDOR_ID_DIS 0x0
#endif


static const struct verbs_match_ent hca_table[] = {
	VERBS_DRIVER_ID(RDMA_DRIVER_UNKNOWN)
}


static const struct verbs_context_ops dis_context_ops = {
    .query_device  	= dis_query_device,
	.query_port    	= dis_query_port,

	.alloc_pd      	= dis_alloc_pd,
	.dealloc_pd    	= dis_dealloc_pd,
	//.free_pd 		= dis_free_pd,

	.reg_mr        	= dis_reg_mr,
	.dereg_mr     	= dis_dereg_mr,

	.create_cq     	= dis_create_cq,
	.poll_cq       	= dis_poll_cq,
	//.req_notify_cq 	= dis_notify_cq,
	//.cq_event      	= dis_cq_event,
	//.resize_cq     	= dis_resize_cq,
	.destroy_cq    	= dis_destroy_cq,

	//.create_srq    	= dis_create_srq,
	//.modify_srq    	= dis_modify_srq,
	//.query_srq     	= dis_query_srq,
	//.destroy_srq   	= dis_destroy_srq,
	//.post_srq_recv 	= dis_post_srq_recv,

	.create_qp     	= dis_create_qp,
	.query_qp      	= dis_query_qp,
	.modify_qp     	= dis_modify_qp,
	.destroy_qp    	= dis_destroy_qp,
	
	.post_send     	= dis_post_send,
	.post_recv     	= dis_post_recv,

	.create_ah     	= dis_create_ah,
	.destroy_ah    	= dis_destroy_ah,
};

static struct verbs_context *dis_alloc_context()
{
	return NULL;
}

static void dis_free_context()
{
}

static struct verbs_device *dis_device_alloc()
{
	return NULL;
}

static void dis_uninit_device()
{
}

static const struct verbs_device_ops dis_device_ops = {
	.name = "dis",
	.match_min_abi_version = 0,
	.match_max_abi_version = DIS_ABI_VERSION,
	.match_table = hca_table,
	.alloc_context = dis_alloc_context,
	.free_context = dis_free_context,
	.alloc_device = dis_device_alloc,
	.uninit_device  = dis_uninit_device,
};
PROVIDER_DRIVER(dis, dis_device_ops);
