// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// March 8, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

unsigned long SW = 0;
unsigned char start = 0;

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// **************Sound_Init*********************
// Initialize SysTick periodic interrupts
// Input: none
// Output: none
// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	unsigned long volatile delay;

	SYSCTL_RCGC2_R |= 0x01; // activate port A
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTA_AMSEL_R &= ~0x0C; // no analog
	GPIO_PORTA_PCTL_R &= ~0x0000FF00; // regular function
	GPIO_PORTA_DIR_R &= ~0x08; // PA3 as input
	GPIO_PORTA_DIR_R |= 0x04; // PA2 as output
	GPIO_PORTA_AFSEL_R &= ~0x0C; // disable alt funct on pa3 pa2
	GPIO_PORTA_DEN_R |= 0x0C; // enable digital i/o pa2 pa3
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_RELOAD_R = 90908; // reload value = 80 MHz x 1.13636 ms
	NVIC_ST_CURRENT_R = 0; // any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0
	NVIC_ST_CTRL_R = 0x07; // enable with core clock and interrupts
	EnableInterrupts();
} 
// Interrupt service routine
// Executed every 880Hz
void SysTick_Handler(void){
  if (start)
		GPIO_PORTA_DATA_R ^= 0x04;     // toggle PA2 440 Hz tone output
	else
		//turn off tone
		GPIO_PORTA_DATA_R &= ~0x04; //make PA2 output low - tone off
}


int main(void) {
  unsigned char toggle = 0;
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2, ScopeOn); // set up system and 80 MHz clock
  Sound_Init(); // initialize sound with 90908 count
  
  while (1) {
    SW = GPIO_PORTA_DATA_R & 0x08; // read PA3 switch status
		
		if (SW == 0x08 && toggle == 1) { // switch pressed, toggle = 1
      start = 1; // enable sound toggling
      while (GPIO_PORTA_DATA_R & 0x08); // wait until switch is released
    } 
    else if (SW == 0x00 && toggle == 1) { // switch released, toggle = 1
      start = 1; // Keep toggling even when the switch is released
      while (!(GPIO_PORTA_DATA_R & 0x08)); // wait until switch is pressed
      toggle = 0; // reset toggle for next cycle
    }
    else if (SW == 0x08 && toggle == 0) { // switch pressed, togglt = 0
      GPIO_PORTA_DATA_R = 0x00; // turn off output, quiet mode
      start = 0;
      while (GPIO_PORTA_DATA_R & 0x08); // wait until switch is released
    } 
    else if (SW == 0x00 && toggle == 0) { // switch released, toggle = 0
      start = 0; // remain in quiet mode
      while (!(GPIO_PORTA_DATA_R & 0x08)); // wait until switch is pressed
      toggle = 1;  // update toggle for next loop
    }
  }
}
