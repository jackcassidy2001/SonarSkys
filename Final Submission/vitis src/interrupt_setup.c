#include "interrupt_setup.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "fifo.h"

static XScuGic intc;
extern int fifo_full_flag;

// interrupt service routine for IRQ_F2P[0:0]
void isr0(void *intc_inst_ptr) {
	xil_printf("\n\r------------------------------------------\n\r");
    xil_printf("isr0 called\n\r");
    unsigned int fifo_status = fifo_get_int_status();
    xil_printf("------------------------------------------\n\r");
    if ((fifo_status & FIFO_TC) != 0) {
        xil_printf("Transmit complete\n\r");
    }
    if ((fifo_status & FIFO_RC) != 0) {
        xil_printf("Receive complete\n\r");
    }
    if ((fifo_status & FIFO_TFPE) != 0) {
        xil_printf("Transmit FIFO programmable empty\n\r");
    }
    if ((fifo_status & FIFO_TFPF) != 0) {
        xil_printf("Transmit FIFO programmable full\n\r");
        fifo_full_flag = 0;
    }
    if ((fifo_status & FIFO_RFPE) != 0) {
        xil_printf("Receive FIFO programmable empty\n\r");
    }
    if ((fifo_status & FIFO_RFPF) != 0) {
        xil_printf("Receive FIFO programmable full\n\r");
    }
    if ((fifo_status & FIFO_RRC) != 0) {
        xil_printf("Receive reset complete\n\r");
    }
    if ((fifo_status & FIFO_TRC) != 0) {
        xil_printf("Transmit reset complete\n\r");
    }
    if ((fifo_status & FIFO_TSE) != 0) {
        xil_printf("Transmit size error\n\r");
    }
    if ((fifo_status & FIFO_TPOE) != 0) {
        xil_printf("Transmit packet overrun error\n\r");
    }
    if ((fifo_status & FIFO_RPUE) != 0) {
        xil_printf("Receive packet underrun error\n\r");
    }
    if ((fifo_status & FIFO_RPORE) != 0) {
        xil_printf("Receive packet overrun read error\n\r");
    }
    if ((fifo_status & FIFO_RPURE) != 0) {
        xil_printf("Receive packet underrun read error\n\r");
    }
    xil_printf("------------------------------------------\n\r");

    //do{
    //	fifo_clr_ints(0xFFFFFFFF);
    //}while(fifo_get_int_status());
}


// sets up the interrupt system and enables interrupts for IRQ_F2P[1:0]
int setup_interrupt_system() {

    int result;
    XScuGic *intc_instance_ptr = &intc;
    XScuGic_Config *intc_config;

    // get config for interrupt controller
    intc_config = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    if (NULL == intc_config) {
        return XST_FAILURE;
    }

    // initialize the interrupt controller driver
    result = XScuGic_CfgInitialize(intc_instance_ptr, intc_config, intc_config->CpuBaseAddress);

    if (result != XST_SUCCESS) {
        return result;
    }

    // set the priority of IRQ_F2P[0:0] to 0xA0 (highest 0xF8, lowest 0x00) and a trigger for a rising edge 0x3.
    XScuGic_SetPriorityTriggerType(intc_instance_ptr, INTC_INTERRUPT_ID_0, 0xA0, 0x3);

    // connect the interrupt service routine isr0 to the interrupt controller
    result = XScuGic_Connect(intc_instance_ptr, INTC_INTERRUPT_ID_0, (Xil_ExceptionHandler)isr0, (void *)&intc);

    if (result != XST_SUCCESS) {
        return result;
    }

    // enable interrupts for IRQ_F2P[0:0]
    XScuGic_Enable(intc_instance_ptr, INTC_INTERRUPT_ID_0);

    // initialize the exception table and register the interrupt controller handler with the exception table
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, intc_instance_ptr);

    // enable non-critical exceptions
    Xil_ExceptionEnable();

    xil_printf("ISR Connected & Enabled\n\r");
    return XST_SUCCESS;
}
