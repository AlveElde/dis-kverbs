#ifndef __DIS_SCI_IF_H__
#define __DIS_SCI_IF_H__

#include "dis_verbs.h"

int dis_sci_if_create_msq(struct dis_wq *wq);
void dis_sci_if_remove_msq(struct dis_wq *wq);
int dis_sci_if_connect_msq(struct dis_wq *wq);
void dis_sci_if_disconnect_msq(struct dis_wq *wq);
int dis_sci_if_send_v_msg(struct dis_wqe *wqe);
int dis_sci_if_receive_v_msg(struct dis_wqe *wqe);

#endif /* __DIS_SCI_IF_H__ */