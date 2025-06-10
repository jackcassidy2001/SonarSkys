#include "fifo.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "limits.h"
#include "minfft.h"
#include "tone.h"
#include <unistd.h>

volatile int fifo_full_flag = 0;

unsigned int fifo_get_int_status() {
    return Xil_In32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_INT_STATUS_OFFSET);
}

void fifo_clr_ints(unsigned int ints) {
    Xil_Out32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_INT_STATUS_OFFSET, ints);
}

int fifo_get_en_int() {
    return Xil_In32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_INT_EN_OFFSET);
}

void fifo_set_en_int(int interrupts) {
    Xil_Out32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_INT_EN_OFFSET, interrupts);
}

int fifo_get_tx_vacancy() {
    return Xil_In32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_TX_VACANCY);
}

int fifo_get_rv_occupancy() {
    return Xil_In32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_RV_OCCUPANCY);
}

void fifo_tx_wr_data_blk(int *data, int num_samples, int *index, int buffer_len) {
    for (int i = 0; i < num_samples; i++) {
        Xil_Out32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_WRITE_PORT_OFFSET, (int)data[*index]);
        //xil_printf("%d\n\r", *index);
        *index = (*index + 1) % buffer_len; // Calculate the current index with wrapping
    }
}

int fifo_rv_rd_len() {
	return Xil_In32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_RV_LEN_BYTES);
}

void fifo_rv_rd_data(int *buffer,int num_samples, int *buffer_index, int buffer_length){
	//xil_printf("getting %d\n\r", num_samples);
	for(int i = 0; i < num_samples; i++){
		//fifo_rv_rd_len();
		buffer[*buffer_index] = (int)(0.5 *Xil_In32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_READ_PORT_OFFSET));
		//xil_printf("Val - %d\n\r", buffer[*buffer_index]);
		*buffer_index = (*buffer_index + 1) % buffer_length;
	}
	return;
}

int fifo_rv_rd_fft(minfft_real *buffer1, minfft_real *buffer2, int num_samples, int *buffer1_index, int *buffer2_index, int buffer_length, int *buf_sel, int *buf_ready, int *rv_buffer, int *rv_index, int rv_length){
	int start_buffer = *buf_sel;
	*buf_ready = 0;
	for(int i = 0; i < num_samples; i++){
		if(!*buf_sel){
			buffer1[*buffer1_index] = (minfft_real)rv_buffer[*rv_index];
			*buffer1_index = (*buffer1_index + 1) % buffer_length;
            if (*buffer1_index == 0) {
            	xil_printf("Switching to buffer 2 \n\r");
                *buf_sel = 1; // Switch buffer when buffer1 is full
                *buf_ready = 1;
            }
		}else{
			buffer2[*buffer2_index] = (minfft_real)rv_buffer[*rv_index];
			*buffer2_index = (*buffer2_index + 1) % buffer_length;
            if (*buffer2_index == 0) {
            	xil_printf("Switching to buffer 1 \n\r");
                *buf_sel = 0; // Switch buffer when buffer2 is full
                *buf_ready = 1;
            }
		}
		*rv_index = (*rv_index + 1) % rv_length;
	}

	return start_buffer;
}

void fifo_tx_wr_data(int data) {
	Xil_Out32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_WRITE_PORT_OFFSET, data);
}

void fifo_tx_strt_len(int tx_bytes) {
	//xil_printf("tx_bytes %d \n\r", tx_bytes);
    Xil_Out32(XPAR_AXI_FIFO_MM_S_0_BASEADDR + FIFO_TX_LEN_BYTES, tx_bytes);
}

int fifo_tx_fill_full(int *data, int *data_index, int data_len) {
    int num_writes = fifo_get_tx_vacancy(); // Initial number of writes based on available space in FIFO
    xil_printf("num writes = %d\n\r", num_writes);
    int total_writes = num_writes;
    int index = *data_index; // Current index in the data array

    // Write data until FIFO is full
    while (num_writes > 0 && !fifo_full_flag) {
        // Write data to FIFO
    	//xil_printf("%d : %d\n\r", index,  data[index]);
        fifo_tx_wr_data(data[index]);

        // Increment data index and wrap back to zero if it exceeds data length
        index++;
        if (index >= data_len) {
            index = 0;
        }

        num_writes--; // Decrement the number of remaining writes
    }
    fifo_full_flag = 0;
    // Update the data index pointer
    *data_index = index;
    xil_printf("Times Written: %d\n\r", total_writes - num_writes);

    return total_writes - num_writes;
}
