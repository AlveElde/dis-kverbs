#ifndef __SCI_IF_H__
#define __SCI_IF_H__

#include "../driver/dis_verbs.h"

int sci_if_create_msq(struct sci_if_msq *msq);
void sci_if_remove_msq(struct sci_if_msq *msq);
int sci_if_connect_msq(struct sci_if_msq *msq);
void sci_if_disconnect_msq(struct sci_if_msq *msq);
int sci_if_send_v_msg(struct sci_if_v_msg *msg);
int sci_if_receive_v_msg(struct sci_if_v_msg *msg);

#endif /* __SCI_IF_H__ */