#include <msp430.h>
char R = 0;                                 //Red LED bit
char G = 0;                                 //Green LED bit
char B = 0;                                 //Blue LED bit
int NRB = 0;                                //Number of received bits
int BIP = 0;                                //bits in package
char message [80];

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
// CCRs stuff
  P1SEL |= BIT2+BIT3+BIT4;                  // P1.2 and P1.3 and P1.4 CCR stuff
  P1DIR |= BIT2+BIT3+BIT4;                  // P1.2 and P1.3 and P1.4 to gpio/CCRs

//Timer Stuff for led
  TA0CCR0 = 512-1;                          // PWM Period is set to 511
  TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
  TA0CCR1 = 0;                              // CCR1 PWM initialization duty cycle
  TA0CCTL2 = OUTMOD_7;                      // CCR2 reset/set
  TA0CCR2 = 0;                              // CCR2 PWM initialization duty cycle
  TA0CCTL3 = OUTMOD_7;                      // CCR3 reset/set
  TA0CCR3 = 255;                              // CCR3 PWM initialization duty cycle
  TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR

//Timer interrupt
  TA1CCTL0 = CCIE;                        // CCR0 interrupt enabled
  TA1CCR0 = 10000;                          //Aclk runs at 10 hz maybe
  TA1CTL = TASSEL_1 + MC_1;

//UART Jawn

  P3SEL |= BIT3+BIT4;                       // P3.3,4 = USCI_A0 TXD/RXD
  UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA1CTL1 |= UCSSEL_2;                     // SMCLK
  UCA1BR0 = 6;                              // 1MHz 115200 (see User's Guide)
  UCA1BR1 = 0;                              // 1MHz 115200
  UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
  UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
                                            // Enable USCI_A0 RX interrupt

  __bis_SR_register(LPM0_bits + GIE);             // Enter LPM0
  __no_operation();                         // For debugger
}
#pragma vector=USCI_A1_VECTOR               //A1 is
__interrupt void USCI_A1_ISR(void)
{
    switch(NRB)
        {
        case 0:
            BIP = UCA1RXBUF;                //the first input will be bits in the package. Set to BIP and increment NRB
            NRB++;
        case 1:
            R = UCA1RXBUF;                                  //the second input is R value. Set to R. Increment NRB
            NRB++;
        case 2:
            G = UCA1RXBUF;                                  //the third input is G value. Set to G. Increment NRB
            NRB++;
        case 3:
            B = UCA1RXBUF;                                  //the fourth input is B value. Set to B. Increment NRB
            NRB++;
        }
    if(NRB > 3 && NRB < BIP)
        {

            message[NRB - 3] = UCA1RXBUF;       //the fifth to BIP-1 values are not for us. Set it to message. Increment NRB every time
            NRB++;
        }
    if(NRB = BIP)
        {
        UCA1TXBUF = (message + (BIP-3));        //add BIP-3 to the message, send the message
        UCA1IFG &= 0;                                               //clear the interrupt flag
        }
}

#pragma vector = TIMER1_A0_VECTOR           //Timer counts
__interrupt void TA1_ISR(void)
    {//Fetch LED values from uart
    TA0CCR1 = R;                            // CCR1 PWM duty cycle
    TA0CCR2 = G;                            // CCR2 PWM duty cycle
    TA0CCR3 = B;                            // CCR3 PWM duty cycle
    }
