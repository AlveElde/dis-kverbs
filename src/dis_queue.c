#include <linux/slab.h>

#include "dis_queue.h"

struct dis_queue *dis_create_queue(int cap, size_t elem_size)
{
    struct dis_queue* queue;

    queue = kmalloc(sizeof(struct dis_queue), GFP_KERNEL);
    if(!queue) {
        printk(KERN_INFO "dis_create_queue failed.\n");
        return NULL;
    }

    queue->cap = cap;
    queue->elem_size = elem_size;
    queue->buf = kmalloc(queue->cap * queue->elem_size, GFP_KERNEL);
    if(!queue->buf) {
        printk(KERN_INFO "dis_create_queue failed.\n");
        return NULL;
    }

    return queue;
}

void dis_destroy_queue(struct dis_queue* queue)
{
    kfree(queue->buf);
    kfree(queue);
}