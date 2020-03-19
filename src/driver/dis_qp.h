#ifndef __DIS_QP_H__
#define __DIS_QP_H__

#include "dis_verbs.h"

#define DIS_QP_SLEEP_MS_INITIAL     800
#define DIS_QP_SLEEP_MS_MAX         1000
#define DIS_QP_SLEEP_MS_INCREASE    10

int dis_qp_init(struct dis_wq *wq);
int dis_qp_notify(struct dis_wq *wq);
void dis_qp_exit(struct dis_wq *wq);

#endif /* __DIS_QP_H__ */