#ifndef __DIS_MAIN_H__
#define __DIS_MAIN_H__

#include <rdma/ib_verbs.h>
#include <rdma/ib_mad.h>


struct dis_device {
    struct ib_device ibdev;
};


#endif /* __DIS_MAIN_H__ */