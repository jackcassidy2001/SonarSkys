#ifndef FIFO_H
#define FIFO_H

#include "minfft.h"

#define XPAR_AXI_FIFO_MM_S_0_BASEADDR 0x43C00000U
#define FIFO_INT_EN_OFFSET 0x04 // interrupt enable
#define FIFO_INT_STATUS_OFFSET 0x0 // interrupt status
#define FIFO_WRITE_PORT_OFFSET 0x10 // write data
#define FIFO_READ_PORT_OFFSET 0x20
#define FIFO_TX_LEN_BYTES 0x14 // write data first. only write length when data ready
#define FIFO_RV_LEN_BYTES 0x24 // read len before data. only read when data available
#define FIFO_TX_VACANCY 0xC // # of 32 bit slots available
#define FIFO_RV_OCCUPANCY 0x1C // # of 32 bits slots in use

// Masks for FIFO interrupts
#define FIFO_RFPE           (1U << 19)
#define FIFO_RFPF           (1U << 20)
#define FIFO_TFPE           (1U << 21)
#define FIFO_TFPF           (1U << 22)
#define FIFO_RRC            (1U << 23)
#define FIFO_TRC            (1U << 24)
#define FIFO_TSE            (1U << 25)
#define FIFO_RC             (1U << 26)
#define FIFO_TC             (1U << 27)
#define FIFO_TPOE           (1U << 28)
#define FIFO_RPUE           (1U << 29)
#define FIFO_RPORE          (1U << 30)
#define FIFO_RPURE          (1U << 31)

#define FIFO_SIZE 508
#define FIFO_REC_MIN 10

void fifo_clr_ints(unsigned int ints);
unsigned int fifo_get_int_status();
void fifo_set_en_int(int interrupts);
int fifo_get_en_int();
int fifo_get_tx_vacancy();
int fifo_get_rv_occupancy();
int fifo_rv_rd_len();
void fifo_rv_rd_data(int *buffer,int num_samples, int *buffer_index, int buffer_length);
int fifo_rv_rd_fft(minfft_real *buffer1, minfft_real *buffer2, int num_samples, int *buffer1_index, int *buffer2_index, int buffer_length, int *buf_sel, int *buf_ready, int *rv_buffer, int *rv_index, int rv_length);
void fifo_tx_wr_data(int data);
void fifo_tx_wr_data_blk(int *data, int num_samples, int *index, int buffer_len);
void fifo_tx_strt_len(int tx_bytes);
int fifo_tx_fill_full(int *data, int *data_index, int data_len);

#endif /* FIFO_H */
