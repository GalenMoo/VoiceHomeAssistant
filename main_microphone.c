#include "msp.h"

void UART0_init(void);
void UART2_init(void);
int flag;

int main(void) {

    CS->KEY = CS_KEY_VAL;
    CS->CTL0 = 0;
    CS->CTL0 = CS_CTL0_DCORSEL_1;
    CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
    CS->KEY = 0;

    UART0_init();
    UART2_init();
    flag = 1;
    WDT_A->CTL = WDT_A_CTL_PW |             // Stop WDT
                 WDT_A_CTL_HOLD;

    // GPIO Setup
    P1->OUT &= ~BIT0;                       // Clear LED to start
    P1->DIR |= BIT0;                        // Set P1.0/LED to output
    P4->SEL1 |= BIT7;                       // Configure P4.7 for ADC
    P4->SEL0 |= BIT7;

    /* initialize P2.2-P2.0 for tri-color LEDs */
    P2->SEL1 &= ~7;           /* configure P2.2-P2.0 as simple I/O */
    P2->SEL0 &= ~7;
    P2->DIR |= 7;             /* P2.2-2.0 set as output */

    // Enable global interrupt
    __enable_irq();

    // Enable ADC interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
    // Enable UART interrupt
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

    // Sampling time, S&H=16, ADC14 on
    ADC14->CTL0 = ADC14_CTL0_SHT0_4 | ADC14_CTL0_SHP | ADC14_CTL0_ON;
    ADC14->CTL1 = ADC14_CTL1_RES_3;         // Use sampling timer, 14-bit conversion results

    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_6;   // A6 ADC input select; Vref=AVCC
    ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt

    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;   // Wake up on exit from ISR

    //3 MHz clk, 64 clk cycles per sample results in 46.8KHz sampling rate

    while (1)
    {
        if (flag == 1){
            ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
            flag = 0;
        }

        __sleep();

        __no_operation();                   // For debugger
    }
}

// ADC14 interrupt service routine
void ADC14_IRQHandler(void) {
    unsigned char ADC = ADC14->MEM[0] >> 6; // grabs upper 8 bits to transmit

    while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
      EUSCI_A0->TXBUF = ADC;
    flag = 1;
 }

void UART0_init(void) {
    EUSCI_A0->CTLW0 |= 1;     /* put in reset mode for config */
    EUSCI_A0->MCTLW = 0;      /* disable oversampling */
    EUSCI_A0->CTLW0 = 0x0081; /* 1 stop bit, no parity, SMCLK, 8-bit data */
    EUSCI_A0->BRW = 26;       /* 3000000 / 115200 = 26 */
    P1->SEL0 |= 0x0C;         /* P1.3, P1.2 for UART */
    P1->SEL1 &= ~0x0C;
    EUSCI_A0->CTLW0 &= ~1;    /* take UART out of reset mode */
    EUSCI_A0->IE |= 1;        /* enable receive interrupt */
}

void UART2_init(void) {
    EUSCI_A2->CTLW0 |= 1;     /* put in reset mode for config */
    EUSCI_A2->MCTLW = 0;      /* disable oversampling */
    EUSCI_A2->CTLW0 = 0x0081; /* 1 stop bit, no parity, SMCLK, 8-bit data */
    EUSCI_A2->BRW = 312;
    P3->SEL0 |= 0x0C;         /* P3.3, P3.2 for UART */
    P3->SEL1 &= ~0x0C;
    EUSCI_A2->CTLW0 &= ~1;    /* take UART out of reset mode */
}

void EUSCIA0_IRQHandler(void) {
    if (EUSCI_A0->RXBUF == 'N'){    //N for on
        while(!(EUSCI_A2->IFG & EUSCI_A_IFG_TXIFG));
          EUSCI_A2->TXBUF = 'Y';   //Y for yes to turn light on
    }
    else if (EUSCI_A0->RXBUF == 'F'){    //F for off
        while(!(EUSCI_A2->IFG & EUSCI_A_IFG_TXIFG));
          EUSCI_A2->TXBUF = 'N';   //N for no to turn light off
    }
}
