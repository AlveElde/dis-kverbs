#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>

MODULE_DESCRIPTION("DIS Device");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

#define DIS_ROPCIE_NAME "dis-ropcie"

extern struct bus_type dis_bus_type;
extern struct device dis_bus_dev;
extern struct device dis_dev_drv;

static void dis_dev_release(struct device *dev)
{
    printk(KERN_INFO "dis_dev_release complete.\n");
}

struct device dis_dev = {
	.init_name = DIS_ROPCIE_NAME,
    .bus = &dis_bus_type,
	.parent = &dis_bus_dev,
	.release = dis_dev_release,
};

static int __init dis_device_init(void)
{
	int ret;
	printk(KERN_INFO "dis_device_init start.\n");

	ret = device_register(&dis_dev);
	if(ret) {
		printk(KERN_INFO "device_register failed!\n");
		return -1;
	}
	printk(KERN_INFO "dis-dev registered.\n");

	printk(KERN_INFO "dis_device_init complete.\n");
    return 0;
}

static void __exit dis_device_exit(void)
{
	printk(KERN_INFO "dis_device_exit start.\n");

	device_unregister(&dis_dev);
	printk(KERN_INFO "dis-dev unregistered.\n");

	printk(KERN_INFO "dis_device_exit complete.\n");
}

module_init(dis_device_init);
module_exit(dis_device_exit);
