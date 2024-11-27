// DAC.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Implementation of the 4-bit digital to analog converter
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Port B bits 3-0 have the 4-bit DAC

#include "DAC.h"
#include "..//tm4c123gh6pm.h"

// **************DAC_Init*********************
// Initialize 4-bit DAC 
// Input: none
// Output: none
void DAC_Init(void) {
  // initialize port B for DAC output (PB3-0)
  GPIO_PORTB_AMSEL_R &= ~0x0F;     // disable analog function on PB3-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // set PB3-0 to regular GPIO
  GPIO_PORTB_DIR_R |= 0x0F;        // set PB3-0 as output
  GPIO_PORTB_AFSEL_R &= ~0x0F;     // set PB3-0 to regular function
  GPIO_PORTB_DEN_R |= 0x0F;        // enable digital on PB3-0
  GPIO_PORTB_DR8R_R |= 0x0F;       // drive up to 8mA on PB3-0

  // initialize port D for input (PD3)
  GPIO_PORTD_AMSEL_R &= ~0x04;     // disable analog function on PD3
  GPIO_PORTD_PCTL_R &= ~0x0000F00F; // set PD3 to regular GPIO
  GPIO_PORTD_DIR_R &= ~0x04;       // set PD3 as input
  GPIO_PORTD_AFSEL_R &= ~0x04;     // set PD3 to regular function
  GPIO_PORTD_DEN_R |= 0x04;        // enable digital on PD3
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Output: none
void DAC_Out(unsigned long data){
  GPIO_PORTB_DATA_R = data;
}
