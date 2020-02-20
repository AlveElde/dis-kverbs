#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>


#define BUS_NAME 			"dis-bus"
#define DIS_BUS_DEV_NAME 	"dis_bus_device"

MODULE_DESCRIPTION("DIS Bus");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

// BUS
static int bus_match(struct device *dev, struct device_driver *driver)
{
	printk(KERN_INFO "dis_bus_type match.\n");
	return !strncmp(dev_name(dev), driver->name, strlen(driver->name));	
}

struct bus_type dis_bus_type = {
    .name = BUS_NAME,
    .match = bus_match,
};

// BUS DEVICE
static void bus_dev_release(struct device *dev)
{
    printk(KERN_INFO "dis_bus_dev release.\n");
}

struct device dis_bus_dev = {
    .init_name = DIS_BUS_DEV_NAME,
	.release = bus_dev_release,
};

// INIT
static int __init dis_bus_init(void)
{
	int ret;
	printk(KERN_INFO "dis_bus_init start.\n");

    // BUS
    ret = bus_register(&dis_bus_type);
    if(ret) {
        printk(KERN_INFO "dis_bus_type failed!\n");
        return -1;
    }
    printk(KERN_INFO "dis_bus_type registered.\n");

	// BUS DEVICE
	ret = device_register(&dis_bus_dev);
    if (ret) {
		printk(KERN_INFO "bus device_register failed!\n");
		bus_unregister(&dis_bus_type);
        return -1;
    }
	printk(KERN_INFO "dis_bus_dev registered.\n");

	printk(KERN_INFO "dis_bus_init complete.\n");
    return 0;
}

// EXIT
static void __exit dis_bus_exit(void)
{
	printk(KERN_INFO "dis_bus_exit start.\n");

    // BUS DEVICE
	device_unregister(&dis_bus_dev);
	printk(KERN_INFO "dis_bus_dev unregistered.\n");

	// BUS
	bus_unregister(&dis_bus_type);
	printk(KERN_INFO "dis_bus_type unregistered.\n");

	printk(KERN_INFO "dis_bus_exit complete.\n");
}

module_init(dis_bus_init);
module_exit(dis_bus_exit);

EXPORT_SYMBOL(dis_bus_type);
EXPORT_SYMBOL(dis_bus_dev);