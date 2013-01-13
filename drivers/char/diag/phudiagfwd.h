#ifndef PHUDIAGFWD_H
#define PHUDIAGFWD_H
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>

#define PHU_DIAG_IN_BUF_SIZE 1024*128

#define PHU_DIAG_OUT_BUF_SIZE 1024*32
#define PHU_DIAG_SMD_BUF_MAX 1024*4
#define PHU_DIAG_READ_WRITE_BUF_SIZE 1024*4
#define PHU_DIAG_SEND_PACKET_MAX_SIZE 1024*4
#define PHU_MAX_BUF_SIZE     1024*32

struct phudiag_buf{
	int data_length;
	int used;
	int buf_size;
	uint8_t buf[0];
};

struct phudiag_ring_buf{
	uint8_t * start;
	uint8_t * end;
	uint8_t * edge;
	int buf_size;
	uint8_t buf[0];
};


struct phudiag_ring_buf * phudiag_ring_buf_malloc(int buf_size);
void phudiag_ring_buf_free(struct phudiag_ring_buf *ring_buf);
void phudiag_ring_buf_clear(struct phudiag_ring_buf *ring_buf);

int phudiagfwd_ring_buf_is_empty(struct phudiag_ring_buf *ring_buf);

int phudiagfwd_ring_buf_set_data_before_process(struct phudiag_ring_buf *ring_buf ,int data_length);
int phudiagfwd_ring_buf_get_data_before_process(struct phudiag_ring_buf *ring_buf,int buf_size);
int phudiagfwd_ring_buf_get_data_after_process(struct phudiag_ring_buf *ring_buf,int read_length);

int phudiagfwd_user_get_data(char __user *buf, int buf_size);
void phudiagfwd_read_data_from_smd(void);
int phudiagfwd_user_set_data(const char __user * data,int data_length);
void phudiagfwd_write_to_smd_work_fn(struct work_struct *work);

#endif
