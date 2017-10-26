#include <msp430.h>

int counter = 0;                                            //counter for UART bit counting

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

   //LED BIT SETUP
    P3DIR |= (BIT4 + BIT6 + BIT5);                          // Set P3.4, P3.5, and P3.6 to output direction
    P3SEL0 |= (BIT4 + BIT6 + BIT5);                         // Sets P3.4, P3.5, and P3.6 to be the output of the CCR1, CCR2, and CCR3
    P3SEL1 &= ~(BIT4 | BIT6 | BIT5);                        // Sets P3.4, P3.5, and P3.6 to be the output of the CCR1, CCR2, and CCR3


   //PWM TIMER SETUP
    TB0CCTL5 = OUTMOD_7;                                    //SET/RESET mode for Red LED
    TB0CCTL3 = OUTMOD_7;                                    //SET/RESET mode for Green LED
    TB0CCTL4 = OUTMOD_7;                                    //SET/RESET mode for Blue LED
    TB0CCR0 = 255;                                          //100% Duty Cycle
    TB0CCR5 = 127;                                          //Red set at 0%
    TB0CCR3 = 127;                                          //Green set at 0%
    TB0CCR4 = 127;                                          //Blue set at 0%
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR | ID_2;         // SMCLK, up mode, clear TBR

    PM5CTL0 &= ~LOCKLPM5;                                   //FR series dude

    // Configure GPIO
    P6SEL1 &= ~(BIT0 | BIT1);
    P6SEL0 |= (BIT0 | BIT1);                                // USCI_A3 UART operation



    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY_H;                                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;                           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;                   // Set all dividers
    CSCTL0_H = 0;                                           // Lock CS registers

    // Configure USCI_A3 for UART mode
    UCA3CTLW0 = UCSWRST;                                    // Put eUSCI in reset
    UCA3CTLW0 |= UCSSEL__SMCLK;                             // UART clock is SMCLK

    UCA3BRW = 52;                                           // Baud Rate
    UCA3MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA3CTLW0 &= ~UCSWRST;                                  // Initialize eUSCI
    UCA3IE |= UCRXIE;                                       // Enable USCI_A3 RX interrupt

    __bis_SR_register(LPM0_bits | GIE);                     // Enter LPM3, interrupts enabled
    __no_operation();                                       // For debugger
}

#pragma vector = USCI_A3_VECTOR
__interrupt void USCI_A3 (void)                             //When UART happens fire and start transmitting and receiving
{
    switch(__even_in_range(UCA3IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;                              //Handles the case where RX = NULL

        case USCI_UART_UCRXIFG:
            switch(counter)
            {
                case 0:
                    while(!(UCA3IFG & UCTXIFG));            //As long as you're not already transmitting, continue, else trap until not transmitting.
                    UCA3TXBUF = UCA3RXBUF - 3;              //Transmits the length of the remaining bytes
                    __no_operation();
                    break;
                case 1:
                    TB0CCR3 = (UCA3RXBUF);                  //sets RED LED
                    break;
                case 2:
                    TB0CCR4 = (UCA3RXBUF);                  //sets GREEN LED
                    break;
                case 3:
                    TB0CCR5 = (UCA3RXBUF);                  //sets BLUE LED
                    break;
                default:
                    while(!(UCA3IFG & UCTXIFG));             //Repeat command from case 0.
                    UCA3TXBUF = UCA3RXBUF;                  //transmit incoming byte to the TX
            }
            if(UCA3RXBUF != 0x0D) {counter++;}              //As long as the byte received is not the final, counter increments and the next LED is set
            else if(UCA3RXBUF == 0x0D){counter = 0;}        //When the final byte is received, reset the counter to start over
            break;
            case USCI_UART_UCTXIFG: break;                  //if we're transmitting nothing else happens

       default: break;                                      // Error check, so we don't break it
    }
}
