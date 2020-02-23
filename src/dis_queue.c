#define DEBUG
#define pr_fmt(fmt) KBUILD_MODNAME ": fn: %s, ln: %d: " fmt, __func__, __LINE__

#include "dis_queue.h"
#include "dis_driver.h"

struct dis_queue *dis_create_queue(struct ib_device *ibdev, int cap, size_t elem_size)
{
    struct dis_queue* queue;

    pr_devel(STATUS_START);

    queue = kmalloc(sizeof(struct dis_queue), GFP_KERNEL);
    if(!queue) {
        dev_err(&ibdev->dev, "kmalloc queue" STATUS_FAIL);
        return NULL;
    }

    queue->cap = cap;
    queue->elem_size = elem_size;
    queue->buf = kmalloc(queue->cap * queue->elem_size, GFP_KERNEL);
    if(!queue->buf) {
        dev_err(&ibdev->dev, "kmalloc buf" STATUS_FAIL);
        return NULL;
    }

    pr_devel(STATUS_COMPLETE);
    return queue;
}

void dis_destroy_queue(struct dis_queue* queue)
{
    pr_devel(STATUS_START);

    kfree(queue->buf);
    kfree(queue);

    pr_devel(STATUS_COMPLETE);
}