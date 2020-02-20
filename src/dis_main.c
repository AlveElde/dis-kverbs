#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>

#include "dis_main.h"
#include "dis_verbs.h"

#define MINOR_BASE  0
#define MINOR_COUNT	255

#define DIS_DRV_NAME 		"dis-device"

#define DIS_ROPCIE_DRV_VERSION "0.0"
#define DIS_ROPCIE_DRV_DESC "Dolphin Interconnect Soulutions RoPCIe Driver"

MODULE_DESCRIPTION(DIS_ROPCIE_DRV_DESC " " DIS_ROPCIE_DRV_VERSION);
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

// BUS
extern struct bus_type dis_bus_type;
extern struct device dis_bus_dev;

// IB DEVICE
static struct dis_device *disdev;

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


// DRIVER
static int driver_probe(struct device *dev)
{
	int ret;

	printk(KERN_INFO "dis-dev probe.\n");

	// IB DEVICE
	disdev = ib_alloc_device(dis_device, ibdev);
	if(!disdev) {
		printk(KERN_INFO "ib_alloc_device failed!\n");
		return -1;
	}
	printk(KERN_INFO "dis-ib-dev allocated.\n");

	// IB REGISTER
	disdev->ibdev.uverbs_cmd_mask = (1ull);
	disdev->ibdev.node_type = RDMA_NODE_UNSPECIFIED;
	disdev->ibdev.phys_port_cnt = 1;
	disdev->ibdev.num_comp_vectors = 1;
	disdev->ibdev.local_dma_lkey = 0;
	disdev->ibdev.node_guid = 1234;
	disdev->ibdev.dev.parent = dev;
	strlcpy(disdev->ibdev.name, "dis", IB_DEVICE_NAME_MAX);
	ib_set_device_ops(&(disdev->ibdev), &disdevops);

	ret = ib_register_device(&(disdev->ibdev), "dis");
	if(ret) {
		printk(KERN_INFO "ib_device_register failed!\n");
		ib_dealloc_device(&(disdev->ibdev));
		return -1;
	}
	printk(KERN_INFO "dis-ib-dev registered.\n");
	
	return 0;
}

static int driver_remove(struct device *dev)
{
	printk(KERN_INFO "dis-dev remove.\n");

	// IB REGISTER
	//TODO: Move to dev_release?
	ib_unregister_device(&(disdev->ibdev));
	ib_dealloc_device(&(disdev->ibdev));
	return 0;
}

struct device_driver drv = {
	.name = DIS_DRV_NAME,
	.bus = &dis_bus_type,
	.probe = driver_probe,
	.remove = driver_remove,
};

// DEVICE
static void dev_release(struct device *dev)
{
    printk(KERN_INFO "dis-dev release.\n");
}

struct device dev = {
	.init_name = DIS_DRV_NAME,
    .bus = &dis_bus_type,
	.parent = &dis_bus_dev,
	.release = dev_release,
};

// static struct attribute *dis_dev_attributes[] = {
// 	&dev_attr_parent.attr,
// 	NULL
// };

// static const struct attribute_group dis_attr_group = {
// 	.attrs = dis_dev_attributes,
// };

// INIT

//TODO: Refactor with goto fall-through unregistration
static int __init dis_init_module(void)
{
	int ret;
	printk(KERN_INFO "dis_init_module start.\n");

	// DRIVER
	ret = driver_register(&drv);
	if(ret) {
		printk(KERN_INFO "driver_register failed!\n");
		return -1;
	}
	printk(KERN_INFO "dis-driver registered.\n");

	// DEVICE
	ret = device_register(&dev);
	if(ret) {
		printk(KERN_INFO "device_register failed!\n");
		driver_unregister(&drv);
		return -1;
	}
	printk(KERN_INFO "dis-dev registered.\n");

	printk(KERN_INFO "dis_init_module complete.\n");
    return 0;
}

// EXIT

static void __exit dis_exit_module(void)
{
	printk(KERN_INFO "dis_exit_module start.\n");
	
	// DEVICE
	device_unregister(&dev);
	printk(KERN_INFO "dis-dev unregistered.\n");

	// DRIVER
	driver_unregister(&drv);
	printk(KERN_INFO "dis-drv unregistered.\n");

	printk(KERN_INFO "dis_exit_module complete.\n");
}

module_init(dis_init_module);
module_exit(dis_exit_module);
