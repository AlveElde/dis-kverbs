#ifndef __DIS_QP_H__
#define __DIS_QP_H__

#include "dis_verbs.h"

int dis_wq_signal(struct dis_wq *diswq, enum dis_thread_flag flag);
int dis_wq_init(struct dis_wq *diswq);
void dis_wq_exit(struct dis_wq *diswq);

#endif /* __DIS_QP_H__ */