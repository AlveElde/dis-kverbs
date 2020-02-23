#define DEBUG
#define pr_fmt(fmt) KBUILD_MODNAME ": fn: %s, ln: %d: " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>

#define BUS_NAME            "dis-bus"
#define DIS_BUS_DEV_NAME    "dis_bus_device"

#define STATUS_START    "Started.\n"
#define STATUS_COMPLETE "Completed.\n"
#define STATUS_FAIL     "Failed.\n"

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
    pr_devel(STATUS_COMPLETE);
}

struct device dis_bus_dev = {
    .init_name = DIS_BUS_DEV_NAME,
	.release = bus_dev_release,
};

static int __init dis_bus_init(void)
{
	int ret;
	pr_devel(STATUS_START);

    ret = bus_register(&dis_bus_type);
    if(ret) {
        pr_devel("dis_bus_type register: " STATUS_FAIL);
        return -1;
    }
    pr_devel("dis_bus_type register: " STATUS_COMPLETE);

	ret = device_register(&dis_bus_dev);
    if (ret) {
		pr_devel("dis_bus_dev register: " STATUS_FAIL);
		bus_unregister(&dis_bus_type);
        return -1;
    }
	pr_devel("dis_bus_dev register: " STATUS_COMPLETE);

	pr_devel(STATUS_COMPLETE);
    return 0;
}

static void __exit dis_bus_exit(void)
{
	pr_devel(STATUS_START);

	device_unregister(&dis_bus_dev);
	pr_devel("dis_bus_dev unregister: " STATUS_COMPLETE);

	bus_unregister(&dis_bus_type);
	pr_devel("dis_bus_type unregister: " STATUS_COMPLETE);

	pr_devel(STATUS_COMPLETE);
}

module_init(dis_bus_init);
module_exit(dis_bus_exit);

EXPORT_SYMBOL(dis_bus_type);
EXPORT_SYMBOL(dis_bus_dev);