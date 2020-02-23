#define DEBUG
#define pr_fmt(fmt) KBUILD_MODNAME ": fn: %s, ln: %d: " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>

#include "dis_driver.h"
#include "dis_verbs.h"

#define MINOR_BASE  0
#define MINOR_COUNT	255

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

	.alloc_pd = dis_alloc_pd,
	.create_cq = dis_create_cq,
	.create_qp = dis_create_qp,
	.dealloc_pd = dis_dealloc_pd,
	.dereg_mr = dis_dereg_mr,
	.destroy_cq = dis_destroy_cq,
	.destroy_qp = dis_destroy_qp,
	.get_dma_mr = dis_get_dma_mr,
	.get_port_immutable = dis_get_port_immutable, 
	.modify_qp = dis_modify_qp,
	.poll_cq = dis_poll_cq,
	.post_recv = dis_post_recv,
	.post_send = dis_post_send,
	.query_pkey = dis_query_pkey,
	.query_port = dis_query_port,
	.query_qp = dis_query_qp,
	.req_notify_cq = dis_req_notify_cq,
    .query_device = dis_query_device,

	INIT_RDMA_OBJ_SIZE(ib_pd, dis_pd, ibpd),
	INIT_RDMA_OBJ_SIZE(ib_ah, dis_ah, ibah),
	INIT_RDMA_OBJ_SIZE(ib_cq, dis_cq, ibcq),
	INIT_RDMA_OBJ_SIZE(ib_ucontext, dis_ucontext, ibucontext),
};

static int dis_driver_probe(struct device *dev)
{
	int ret;

	pr_devel(STATUS_START);

	// if (dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64ULL)) &&
	//     dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32ULL))) {
	// 	dev_err(dev, "Not usable DMA addressing mode\n");
	// 	ret = -EIO;
	// 	goto error_failed_get_cfg;
	// }

	disdev = ib_alloc_device(dis_dev, ibdev);
	if(!disdev) {
		dev_err(dev, "ib_alloc_device " STATUS_FAIL);
		return -1;
	}
	pr_devel("ib_alloc_device " STATUS_COMPLETE);

	disdev->ibdev.uverbs_cmd_mask = (1ull);
	disdev->ibdev.node_type = RDMA_NODE_UNSPECIFIED;
	disdev->ibdev.phys_port_cnt = 1;
	disdev->ibdev.num_comp_vectors = 1;
	disdev->ibdev.local_dma_lkey = 1234;
	disdev->ibdev.node_guid = 1234;
	disdev->ibdev.dev.parent = dev;
	strlcpy(disdev->ibdev.name, "dis", IB_DEVICE_NAME_MAX);
	ib_set_device_ops(&(disdev->ibdev), &disdevops);

	ret = ib_register_device(&(disdev->ibdev), "dis");
	if(ret) {
		dev_err(dev, "ib_register_device " STATUS_FAIL);
		ib_dealloc_device(&(disdev->ibdev));
		return -1;
	}

	pr_devel(STATUS_COMPLETE);
	return 0;
}

static int dis_driver_remove(struct device *dev)
{
	pr_devel(STATUS_START);

	//TODO: Move to dev_release?
	ib_unregister_device(&(disdev->ibdev));
	ib_dealloc_device(&(disdev->ibdev));

	pr_devel(STATUS_COMPLETE);
	return 0;
}

struct device_driver dis_dev_drv = {
	.name = DIS_ROPCIE_NAME,
	.bus = &dis_bus_type,
	.probe = dis_driver_probe,
	.remove = dis_driver_remove,
};

static int __init dis_driver_init(void)
{
	int ret;

	pr_devel(STATUS_START);

	ret = driver_register(&dis_dev_drv);
	if(ret) {
		pr_err("driver_register" STATUS_FAIL);
		return -1;
	}

	pr_devel(STATUS_COMPLETE);
    return 0;
}

static void __exit dis_driver_exit(void)
{
	pr_devel(STATUS_START);

	driver_unregister(&dis_dev_drv);

	pr_devel(STATUS_COMPLETE);
}

module_init(dis_driver_init);
module_exit(dis_driver_exit);

EXPORT_SYMBOL(dis_dev_drv);
