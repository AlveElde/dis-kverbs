#ifndef __DIS_QUEUE_H__
#define __DIS_QUEUE_H__

#include <linux/types.h>

#include <rdma/ib_verbs.h>

struct dis_queue {
    int cap;
    size_t elem_size;
    void* buf;
};

struct dis_queue *dis_create_queue(struct ib_device *ibdev, int cap, size_t elem_size);
void dis_destroy_queue(struct dis_queue* queue);

#endif /* __DIS_QUEUE_H__ */