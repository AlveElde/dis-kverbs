#include "pr_fmt.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include "dis_driver.h"
#include "dis_verbs.h"

#define DIS_ROPCIE_NAME "dis-ropcie"
#define DIS_ROPCIE_DRV_VERSION "0.0"
#define DIS_ROPCIE_DRV_DESC "Dolphin Interconnect Soulutions RoPCIe Driver"

MODULE_DESCRIPTION(DIS_ROPCIE_DRV_DESC " " DIS_ROPCIE_DRV_VERSION);
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

extern struct bus_type dis_bus_type;
extern struct device dis_bus_dev;
static struct dis_dev *disdev;

static const struct ib_device_ops disdevops = {
    .owner = THIS_MODULE,
    .driver_id = RDMA_DRIVER_UNKNOWN,
    .uverbs_abi_ver = 1,

    .alloc_ucontext     = dis_alloc_ucontext,
    .dealloc_ucontext   = dis_dealloc_ucontext,
    .alloc_pd           = dis_alloc_pd,
    .create_cq          = dis_create_cq,
    .create_qp          = dis_create_qp,
    .dealloc_pd         = dis_dealloc_pd,
    .dereg_mr           = dis_dereg_mr,
    .destroy_cq         = dis_destroy_cq,
    .destroy_qp         = dis_destroy_qp,
    .get_dma_mr         = dis_get_dma_mr,
    .get_port_immutable = dis_get_port_immutable, 
    .modify_qp          = dis_modify_qp,
    .poll_cq            = dis_poll_cq,
    .post_recv          = dis_post_recv,
    .post_send          = dis_post_send,
    .query_pkey         = dis_query_pkey,
    .query_port         = dis_query_port,
    .query_qp           = dis_query_qp,
    .req_notify_cq      = dis_req_notify_cq,
    .query_device       = dis_query_device,
    .create_srq         = dis_create_srq,
    .modify_srq         = dis_modify_srq,
    .query_srq          = dis_query_srq,
    .post_srq_recv      = dis_post_srq_recv,
    .destroy_srq        = dis_destroy_srq,

    INIT_RDMA_OBJ_SIZE(ib_pd, dis_pd, ibpd),
    INIT_RDMA_OBJ_SIZE(ib_ah, dis_ah, ibah),
    INIT_RDMA_OBJ_SIZE(ib_cq, dis_cq, ibcq),
    INIT_RDMA_OBJ_SIZE(ib_srq, dis_srq, ibsrq),
    INIT_RDMA_OBJ_SIZE(ib_ucontext, dis_ucontext, ibucontext),
};

static int dis_driver_probe(struct device *dev)
{
    int ret;

    pr_devel(DIS_STATUS_START);

    disdev = ib_alloc_device(dis_dev, ibdev);
    if(!disdev) {
        dev_err(dev, "ib_alloc_device " DIS_STATUS_FAIL);
        return -1;
    }
    pr_devel("ib_alloc_device " DIS_STATUS_COMPLETE);

    disdev->ibdev.uverbs_cmd_mask =
		(1ull << IB_USER_VERBS_CMD_GET_CONTEXT)     |
        (1ull << IB_USER_VERBS_CMD_QUERY_DEVICE)    |
        (1ull << IB_USER_VERBS_CMD_QUERY_PORT)      |
		(1ull << IB_USER_VERBS_CMD_ALLOC_PD)        |
		(1ull << IB_USER_VERBS_CMD_DEALLOC_PD)      |
		// (1ull << IB_USER_VERBS_CMD_REG_MR) |
		// (1ull << IB_USER_VERBS_CMD_DEREG_MR) |
		// (1ull << IB_USER_VERBS_CMD_CREATE_COMP_CHANNEL) |
		(1ull << IB_USER_VERBS_CMD_CREATE_CQ)       |
		(1ull << IB_USER_VERBS_CMD_POLL_CQ)         |
		(1ull << IB_USER_VERBS_CMD_REQ_NOTIFY_CQ)   |
		(1ull << IB_USER_VERBS_CMD_DESTROY_CQ)      |
		(1ull << IB_USER_VERBS_CMD_CREATE_QP)       |
		(1ull << IB_USER_VERBS_CMD_QUERY_QP)        |
		(1ull << IB_USER_VERBS_CMD_MODIFY_QP)       |
		(1ull << IB_USER_VERBS_CMD_DESTROY_QP)      |
		(1ull << IB_USER_VERBS_CMD_POST_SEND)       |
		(1ull << IB_USER_VERBS_CMD_POST_RECV)       |
		(1ull << IB_USER_VERBS_CMD_CREATE_SRQ)      |
		(1ull << IB_USER_VERBS_CMD_MODIFY_SRQ)      |
		(1ull << IB_USER_VERBS_CMD_QUERY_SRQ)       |
		(1ull << IB_USER_VERBS_CMD_POST_SRQ_RECV)   |
		(1ull << IB_USER_VERBS_CMD_DESTROY_SRQ);


    disdev->ibdev.node_type         = RDMA_NODE_RNIC;
    disdev->ibdev.phys_port_cnt     = 1;
    disdev->ibdev.num_comp_vectors  = 1;
    disdev->ibdev.local_dma_lkey    = 1234;
    disdev->ibdev.node_guid         = 1234;
    disdev->ibdev.dev.parent        = dev;
    strlcpy(disdev->ibdev.name, DIS_ROPCIE_NAME, IB_DEVICE_NAME_MAX);
    // strlcpy(disdev->ibdev.node_desc, DIS_ROPCIE_NAME, strlen(DIS_ROPCIE_NAME));
    ib_set_device_ops(&(disdev->ibdev), &disdevops);

    ret = ib_register_device(&(disdev->ibdev), DIS_ROPCIE_NAME);
    if(ret) {
        dev_err(dev, "ib_register_device: " DIS_STATUS_FAIL);
        ib_dealloc_device(&(disdev->ibdev));
        return -1;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static int dis_driver_remove(struct device *dev)
{
    pr_devel(DIS_STATUS_START);

    ib_unregister_device(&(disdev->ibdev));
    ib_dealloc_device(&(disdev->ibdev));

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

struct device_driver dis_dev_drv = {
    .name       = DIS_ROPCIE_NAME,
    .bus        = &dis_bus_type,
    .probe      = dis_driver_probe,
    .remove     = dis_driver_remove,
};

static int __init dis_driver_init(void)
{
    int ret;

    pr_devel(DIS_STATUS_START);

    ret = driver_register(&dis_dev_drv);
    if(ret) {
        pr_err("driver_register" DIS_STATUS_FAIL);
        return -1;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit dis_driver_exit(void)
{
    pr_devel(DIS_STATUS_START);

    driver_unregister(&dis_dev_drv);

    pr_devel(DIS_STATUS_COMPLETE);
}

module_init(dis_driver_init);
module_exit(dis_driver_exit);

EXPORT_SYMBOL(dis_dev_drv);
