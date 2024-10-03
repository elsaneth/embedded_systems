// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// Constant declarations to access port registers using
// from lab 6  but used port E
// https://users.ece.utexas.edu/~valvano/Volume1/E-Book/C6_MicrocontrollerPorts.htm
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port F Clock Gating Control

// ***** 2. Global Declarations Section *****
unsigned long SW1;

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay1ms(unsigned long int);
void PortE_init(void);

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
	unsigned long delay;
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_DIR_R = 0x02; // PE1 is output -> binary: 0000 0010
	GPIO_PORTE_DEN_R |= 0x03; // enable digital pins PE1 and PE0 -> binary: 0000 0011
	GPIO_PORTE_DATA_R |= 0x03; // for accessing ports PE1 and PE0 -> binary: 0000 0011
	
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
		SW1 = GPIO_PORTE_DATA_R&0x01; // read PE0 into SW1
		if(SW1) {
			Delay1ms(100);
			// use red led: 0x02
			GPIO_PORTE_DATA_R^= 0x02; // ^= XOR toggles the state of bit - binary: 0000 0010
		} else {
			GPIO_PORTE_DATA_R |= 0x02; // |= OR ensures the second (index 1) bit is 1 - so red led is ON by default
		}
  }
}

// from lab 7
void Delay1ms(unsigned long msec){
	unsigned long i;
	while (msec > 0) {
		i = 13333;
		while (i > 0) {
			i = i - 1;
		}
		msec = msec - 1;
	}
}

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06
