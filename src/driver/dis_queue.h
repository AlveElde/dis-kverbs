#ifndef __DIS_QUEUE_H__
#define __DIS_QUEUE_H__

#include <rdma/ib_verbs.h>

struct dis_queue {
    u32 max_elem;
    size_t elem_size;
    void* buf;
};

int dis_create_queue(struct dis_queue *queue);
void dis_destroy_queue(struct dis_queue* queue);

#endif /* __DIS_QUEUE_H__ */