#ifndef __DIS_QP_H__
#define __DIS_QP_H__

#include "dis_verbs.h"

#define DIS_QP_SLEEP_MS_MAX 1000
#define DIS_QP_CONNECT_RETRY_MAX 10

int dis_wq_signal(struct dis_wq *diswq, enum dis_wq_flag flag);
int dis_qp_init(struct dis_qp *disqp);
void dis_qp_exit(struct dis_qp *disqp);

#endif /* __DIS_QP_H__ */