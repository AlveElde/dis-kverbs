#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "dis_main.h"
#include "dis_verbs.h"

MODULE_DESCRIPTION(IDS_ROPCIE_DRV_DESC " " IDS_ROPCIE_DRV_VERSION);
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("Proprietary");


static const struct ib_device_ops dis_dev_ops = {
	.owner = THIS_MODULE,
	.driver_id = RDMA_DRIVER_UNKNOWN,
	.uverbs_abi_ver = 1,

	.alloc_pd = dis_alloc_pd,
	.create_ah = dis_create_ah,
	.create_cq = dis_create_cq,
	.create_qp = dis_create_qp,
	.dealloc_pd = dis_dealloc_pd,
	.dereg_mr = dis_dereg_mr,
	.destroy_ah = dis_destroy_ah,
	.destroy_cq = dis_destroy_cq,
	.destroy_qp = dis_destroy_qp,
	.modify_qp = dis_modify_qp,
	.poll_cq = dis_poll_cq,
	.post_recv = dis_post_recv,
	.post_send = dis_post_send,
	.query_port = dis_query_port,
	.query_qp = dis_query_qp,
	
    .query_device = dis_query_device,

	// Additional kernel verbs
	// .alloc_ucontext = dis_alloc_ucontext,
	// .dealloc_ucontext = dis_dealloc_ucontext,
	// .get_link_layer = dis_link_layer,

	//  Unsure
	// dis_query_gid
	// dis_mmap
	// dis_reg_user_mr,
	// dis_alloc_mr

	// Not in kernel
	// .reg_mr = dis_reg_mr,

	// INIT_RDMA_OBJ_SIZE(ib_ah, dis_ah, ibah),
	// INIT_RDMA_OBJ_SIZE(ib_cq, dis_cq, ibcq),
	// INIT_RDMA_OBJ_SIZE(ib_pd, dis_pd, ibpd),
	// INIT_RDMA_OBJ_SIZE(ib_ucontext, dis_ucontext, ibucontext),
};

// static int dis_register_device(struct dis_dev *dev)
// {
// 	int ret;
// }

static int __init dis_init_module(void)
{
	printk(KERN_INFO "Hello, world from DIS driver\n");
    return 0;
}

static void __exit dis_exit_module(void)
{
}

module_init(dis_init_module);
module_exit(dis_exit_module);
