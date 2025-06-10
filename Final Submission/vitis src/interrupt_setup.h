#ifndef INTERRUPT_SETUP_H
#define INTERRUPT_SETUP_H

#include "xscugic.h"
#include "xparameters.h"

#define INTC_INTERRUPT_ID_0 XPAR_FABRIC_AXI_FIFO_MM_S_0_INTERRUPT_INTR // IRQ_F2P[0:0]

int setup_interrupt_system();

#endif /* INTERRUPT_SETUP_H */
