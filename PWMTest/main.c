#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    // Configure GPIO
    P1DIR |= BIT4 | BIT5;                   // P3.5 and P3.6 output
    P1SEL0 |= BIT4 | BIT5;                  // P3.5 and P3.6 options select
    P1SEL1 &= ~(BIT4 | BIT5);

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    TB0CCR0 = 1000-1;                       // PWM Period
    TB0CCTL1 = OUTMOD_7;                    // CCR1 reset/set
    TB0CCR1 = 750;                          // CCR1 PWM duty cycle
    TB0CCTL2 = OUTMOD_7;                    // CCR2 reset/set
    TB0CCR2 = 250;                          // CCR2 PWM duty cycle
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR;// SMCLK, up mode, clear TBR

    __bis_SR_register(LPM0_bits);           // Enter LPM0
    __no_operation();                       // For debugger
}
