#ifndef __SCI_IF_H__
#define __SCI_IF_H__

#include "../driver/dis_verbs.h"

int sci_if_create_msq(struct dis_wq *wq);
void sci_if_remove_msq(struct dis_wq *wq);
int sci_if_connect_msq(struct dis_wq *wq);
void sci_if_disconnect_msq(struct dis_wq *wq);
int sci_if_send_v_msg(struct dis_wqe *wqe);
int sci_if_receive_v_msg(struct dis_wqe *wqe);

#endif /* __SCI_IF_H__ */