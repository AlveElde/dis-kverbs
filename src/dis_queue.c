#include "pr_fmt.h"

#include "dis_queue.h"
#include "dis_driver.h"

int dis_create_queue(struct dis_queue* queue)
{
    pr_devel(STATUS_START);

    if(!queue) {
        pr_devel("Queue is NULL: " STATUS_FAIL);
        return -1;
    }

    queue->buf = NULL;

    if(queue->max_elem < 0 || queue->elem_size < 0) {
        pr_devel("Queue dimension(s) negative: " STATUS_FAIL);
        return -1;
    }

    if(queue->max_elem == 0 || queue->elem_size == 0) {
        pr_devel("Queue of 0 size requested: " STATUS_COMPLETE);
        return 0;
    }

    queue->buf = kmalloc(queue->max_elem * queue->elem_size, GFP_KERNEL);
    if(!queue->buf) {
        pr_devel("kmalloc queue: " STATUS_FAIL);
        return -1;
    }

    pr_devel(STATUS_COMPLETE);
    return 0;
}

void dis_destroy_queue(struct dis_queue* queue)
{
    pr_devel(STATUS_START);

    if(!queue) {
        pr_devel("Queue is NULL: " STATUS_FAIL);
        return;
    }

    if(!queue->buf) {
        pr_devel("Queue buf is NULL: " STATUS_FAIL);
        return;
    }

    kfree(queue->buf);
    pr_devel(STATUS_COMPLETE);
}