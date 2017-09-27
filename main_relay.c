//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"

void delayMs(int n);
void UART2_init(void);

void main(void){
    
    UART2_init();

    P2->DIR |= BIT0;

    P3->DIR |= BIT0;
    P3->OUT &= ~BIT0;


    NVIC_SetPriority(EUSCIA0_IRQn, 4); /* set priority to 4 in NVIC */
    NVIC_EnableIRQ(EUSCIA2_IRQn);      /* enable interrupt in NVIC */
    __enable_irq();                    /* global enable IRQs */

    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;   // wake up on exit from ISR

   while(1){

       _sleep();

   }
    
}

void delayMs(int n) {
    int i, j;

    for (j = 0; j < n; j++)
        for (i = 149; i > 0; i--);      /* Delay */
}

void UART2_init(void) {
    EUSCI_A2->CTLW0 |= 1;     /* put in reset mode for config */
    EUSCI_A2->MCTLW = 0;      /* disable oversampling */
    EUSCI_A2->CTLW0 = 0x0081; /* 1 stop bit, no parity, SMCLK, 8-bit data */
    EUSCI_A2->BRW = 312;       /* 3000000 / 9600 = 312 */
    EUSCI_A2->MCTLW =
    P3->SEL0 |= 0x0C;         /* P3.2, P3.3 for UART */
    P3->SEL1 &= ~0x0C;
    EUSCI_A2->CTLW0 &= ~1;    /* take UART out of reset mode */
    EUSCI_A2->IE |= 1;        /* enable receive interrupt */
}

void EUSCIA2_IRQHandler(void) {

                                        /* interrupt flag is cleared by reading RXBUF */

    if (EUSCI_A2->RXBUF == 'Y'){         /* read the receive char and check */

        P3->OUT |= BIT0;                /* interrupt flag is cleared by reading RXBUF */

    }

    else if (EUSCI_A2->RXBUF == 'N'){

        P3->OUT &= ~BIT0;

    }

    else{}
}