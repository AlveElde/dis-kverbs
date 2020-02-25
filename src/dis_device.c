#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>

MODULE_DESCRIPTION("DIS Device");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

#define DIS_ROPCIE_NAME "dis-ropcie"

#define STATUS_START    "Started.\n"
#define STATUS_COMPLETE "Completed.\n"
#define STATUS_FAIL     "Failed.\n"

extern struct bus_type dis_bus_type;
extern struct device dis_bus_dev;
extern struct device dis_dev_drv;

static void dis_dev_release(struct device *dev)
{
    pr_devel(STATUS_COMPLETE);
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

	pr_devel(STATUS_START);

	ret = device_register(&dis_dev);
	if(ret) {
		pr_devel("dis_dev register: " STATUS_FAIL);
		return -1;
	}
	pr_devel("dis_dev register: " STATUS_COMPLETE);

	pr_devel(STATUS_COMPLETE);
    return 0;
}

static void __exit dis_device_exit(void)
{
	pr_devel(STATUS_START);

	device_unregister(&dis_dev);
	pr_devel(STATUS_COMPLETE);

	pr_devel(STATUS_COMPLETE);
}

module_init(dis_device_init);
module_exit(dis_device_exit);
