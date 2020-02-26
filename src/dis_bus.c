#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>

#define BUS_NAME            "dis-bus"
#define DIS_BUS_DEV_NAME    "dis_bus_device"

MODULE_DESCRIPTION("DIS Bus");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static int bus_match(struct device *dev, struct device_driver *driver)
{
	return !strncmp(dev_name(dev), driver->name, strlen(driver->name));	
}

struct bus_type dis_bus_type = {
    .name = BUS_NAME,
    .match = bus_match,
};

static void bus_dev_release(struct device *dev)
{
    pr_devel(DIS_STATUS_COMPLETE);
}

struct device dis_bus_dev = {
    .init_name = DIS_BUS_DEV_NAME,
	.release = bus_dev_release,
};

static int __init dis_bus_init(void)
{
	int ret;
	pr_devel(DIS_STATUS_START);

    ret = bus_register(&dis_bus_type);
    if(ret) {
        pr_devel("dis_bus_type register: " DIS_STATUS_FAIL);
        return -1;
    }
    pr_devel("dis_bus_type register: " DIS_STATUS_COMPLETE);

	ret = device_register(&dis_bus_dev);
    if (ret) {
		pr_devel("dis_bus_dev register: " DIS_STATUS_FAIL);
		bus_unregister(&dis_bus_type);
        return -1;
    }
	pr_devel("dis_bus_dev register: " DIS_STATUS_COMPLETE);

	pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit dis_bus_exit(void)
{
	pr_devel(DIS_STATUS_START);

	device_unregister(&dis_bus_dev);
	pr_devel("dis_bus_dev unregister: " DIS_STATUS_COMPLETE);

	bus_unregister(&dis_bus_type);
	pr_devel("dis_bus_type unregister: " DIS_STATUS_COMPLETE);

	pr_devel(DIS_STATUS_COMPLETE);
}

module_init(dis_bus_init);
module_exit(dis_bus_exit);

EXPORT_SYMBOL(dis_bus_type);
EXPORT_SYMBOL(dis_bus_dev);