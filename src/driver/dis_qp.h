#ifndef __DIS_QP_H__
#define __DIS_QP_H__

#include "dis_verbs.h"

int dis_sq_signal(struct dis_qp *disqp, enum dis_thread_flag flag);
int dis_sq_init(struct dis_qp *disqp);
void dis_sq_exit(struct dis_qp *disqp);
int dis_rq_signal(struct dis_qp *disqp, enum dis_thread_flag flag);
int dis_rq_init(struct dis_qp *disqp);
void dis_rq_exit(struct dis_qp *disqp);

#endif /* __DIS_QP_H__ */