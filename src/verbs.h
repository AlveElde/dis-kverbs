#include <infiniband/driver.h>
#include <infiniband/verbs.h>

// Device verbs.
int dis_query_device(struct ibv_context *uctx, struct ibv_device_attr *attr);           // Used by nccl, srq_pp
int dis_query_port(struct ibv_context *uctx, uint8_t port, struct ibv_port_attr *attr); // Used by nccl, srq_pp

// Protection Domain verbs.
struct ibv_pd *dis_alloc_pd(struct ibv_context *uctx);  // Used by nccl, srq_pp
int dis_dealloc_pd(struct ibv_pd *ibvpd);               // Used by nccl, srq_pp
//int dis_free_pd();

// Memory Region verbs.
struct ibv_mr *dis_reg_mr(struct ibv_pd *ibvpd, void *buf, size_t len, uint64_t hca_va, int ibv_access_flags);  // Used by nccl, srq_pp
int dis_dereg_mr(struct verbs_mr *vmr);                                                                         // Used by nccl, srq_pp

// Completion Queue verbs.
struct ibv_cq *dis_create_cq(struct ibv_context *uctx, int ncqe, struct ibv_comp_channel *ch, int vec);     // Used by nccl, srq_pp
int dis_poll_cq(struct ibv_cq *ibvcq, int nwc, struct ibv_wc *wc);                                          // Used by nccl, srq_pp
//int dis_notify_cq();                                                                                      // Used by srq_pp, not in efa
//int dis_cq_event();                                                                                       // Not in HFIverbs, efa
//int dis_resize_cq();                                                                                      // Not in efa
int dis_destroy_cq(struct ibv_cq *ibvcq);                                                                   // Used by nccl, srq_pp

// Shared Receive Queue verbs.
//truct ibv_srq *dis_create_srq();    // Used by srq_pp, Not in efa
//int dis_modify_srq();               // Not in efa
//int dis_query_srq();                // Not in bnxt_re, cxgb3, efa
//int dis_resize_srq();               // Not in bnxt_re, efa
//int dis_destroy_srq();              // Used by srq_pp, Not in bnxt_re, efa
//int dis_post_srq_recv();            // Used by srq_pp, Not in efa

// Queue pair verbs.
struct ibv_qp *dis_create_qp(struct ibv_pd *ibvpd, struct ibv_qp_init_attr *attr);                                      // Used by nccl, srq_pp
int dis_query_qp(struct ibv_qp *ibvqp, struct ibv_qp_attr *attr, int attr_mask, struct ibv_qp_init_attr *init_attr);    // Used by nccl, srq_pp
int dis_modify_qp(struct ibv_qp *ibvqp, struct ibv_qp_attr *attr, int ibv_qp_attr_mask);                                // Used by nccl, srq_pp
int dis_destroy_qp(struct ibv_qp *ibvqp);                                                                               // Used by nccl, srq_pp

// Posting verbs.
int dis_post_send(struct ibv_qp *ibvqp, struct ibv_send_wr *wr, struct ibv_send_wr **bad);  // Used by nccl, srq_pp
int dis_post_recv(struct ibv_qp *ibvqp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad);  // Used by nccl, srq_pp

// Address Handle verbs.
struct ibv_ah *dis_create_ah(struct ibv_pd *ibvpd, struct ibv_ah_attr *attr);   // Used by ud_pp
int dis_destroy_ah(struct ibv_ah *ibvah);                                       // Used by ud_pp
