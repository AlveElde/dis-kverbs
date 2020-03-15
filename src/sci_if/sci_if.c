#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "sci_if_lib.h"
#include "sci_if_send_queue.h"

MODULE_DESCRIPTION("SCI Lib Interface");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

module_param(local_adapter_no, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(remote_node_id, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(is_initiator, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(local_adapter_no, "");
MODULE_PARM_DESC(remote_node_id, "");
MODULE_PARM_DESC(is_initiator, "");

#define SCIL_INIT_FLAGS 0
#define SCIL_EXIT_FLAGS 0


static int __init sci_if_init(void)
{
    sci_error_t ret;
    pr_devel(DIS_STATUS_START);
    
    ret = SCILInit(SCIL_INIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return -1;
    }

    

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit sci_if_exit(void)
{
    sci_error_t ret;
    pr_devel(DIS_STATUS_START);
    
    ret = SCILDestroy(SCIL_EXIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return;
    }

    pr_devel(DIS_STATUS_COMPLETE);
}

module_init(sci_if_init);
module_exit(sci_if_exit);