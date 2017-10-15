#include <msp430.h>
char message[];
char R = 0;
char G = 0;
char B = 0;
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
// CCRs stuff
  P1DIR |= BIT2+BIT3+BIT4;                  // P1.2 and P1.3 and P1.4 to gpio/CCRs
  P1SEL |= BIT2+BIT3+BIT4;                  // P1.2 and P1.3 and P1.4 CCR stuff

//Timer Stuff for led
  TA0CCR0 = 512-1;                          // PWM Period is set to 511
  TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
  TA0CCR1 = 0;                              // CCR1 PWM initialization duty cycle
  TA0CCTL2 = OUTMOD_7;                      // CCR2 reset/set
  TA0CCR2 = 0;                              // CCR2 PWM initialization duty cycle
  TA0CCTL3 = OUTMOD_7;                      // CCR3 reset/set
  TA0CCR3 = 0;                              // CCR3 PWM initialization duty cycle
  TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR

//Timer interrupt
  TA1CCTL0 = CCIE;                        // CCR0 interrupt enabled
  TA1CCR0 = 10000;                          //Aclk runs at 10 hz maybe
  TA1CTL = TASSEL_1 + MC_1;

//UART Jawn

  P3SEL |= BIT3+BIT4;                       // P3.3,4 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
                                            // Enable USCI_A0 RX interrupt

  __bis_SR_register(LPM0_bits);             // Enter LPM0
  __no_operation();                         // For debugger
}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
   switch(R)//Checks to see if Red led is set
   {
   case 0 : //if not set then set it to receiving bit
       R = UCA0RXBUF;
   default //leave statement to set the next color
       break;
   }
   switch(G)//checks if Green LED is set
   {
   case 0 : //if not set
       G = UCA0RXBUF;
   default //if it is then leave
       break;
   }
   switch(B)//checks if blue led is set
   {
   case 0 :
       B = UCA0RXBUF;
   default //if not leave
       break;
   }
}

#pragma vector = TIMER1_A0_VECTOR           //Timer counts
__interrupt void TA1_ISR(void)
    {//Fetch LED values from uart
    TA0CCR1 = R;                            // CCR1 PWM duty cycle
    TA0CCR2 = G;                            // CCR2 PWM duty cycle
    TA0CCR3 = B;
    }
