#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "dis_scilib_if.h"

MODULE_DESCRIPTION("SCI Lib Interface");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static unsigned int local_adapter_no    = 99;
static unsigned int remote_node_id      = 99;
static bool is_responder                = true;

module_param(local_adapter_no, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(remote_node_id, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(is_responder, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

#define SCIL_INIT_FLAGS 0
#define SCIL_EXIT_FLAGS 0

static int __init dis_scilib_if_init(void)
{
    sci_error_t ret;
    pr_devel(DIS_STATUS_START);
    
    ret = SCILInit(SCIL_INIT_FLAGS);
    if(ret != SCI_ERR_OK) {
        pr_devel(DIS_STATUS_FAIL);
        return 0;
    }

    // if(is_responder) {
    //     test_responder(local_adapter_no, remote_node_id);
    // } else {
    //     test_requester(local_adapter_no, remote_node_id);
    // }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit dis_scilib_if_exit(void)
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

module_init(dis_scilib_if_init);
module_exit(dis_scilib_if_exit);