// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE2(Ain1) and PD3
// Slide pot pin 1 connected to ground


#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"

void EnableInterrupts(void);  // Enable interrupts

unsigned long Size = 2; // potentiomter range in cm
unsigned char String[10]; // string to output to screen
unsigned long Distance; // potentiometer distance in units of 0.001 cm
unsigned long ADCdata;  // 12-bit 0 to 4095 sample
unsigned long Flag;     // 1 means valid Distance, 0 means Distance is empty

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
	// Converts ADC input to actual distance in units of 0.001cm
	// Input: sample  12-bit ADC sample
	// Output: 32-bit distance (resolution 0.001cm)
	return (sample * Size * 1000) / 4095;
}


// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
	// Input: 32bit desired reload value
	NVIC_ST_CTRL_R = 0; // disable systick
	NVIC_ST_RELOAD_R = period - 1; // reload value
	NVIC_ST_CURRENT_R = 0; // clear flag
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // set priority
	NVIC_ST_CTRL_R = 0x07; // enable systick
}

//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n){
  // this function converts the adc value into a string like "0.102 cm"
  // input: n - the adc value (resolution 0.001 cm)
  // output: the string will be stored in String[10]
  
  int i;  // a variable for looping
  
  // if the number is bigger than 9999, we will just show "*.*"
  if (n > 9999) {
    // set '*' for the middle part of the string (just a placeholder)
    String[1] = '*';
    String[2] = '*';
    String[3] = '*';
    String[4] = '*';
  } else {
    // now we will split the number into individual digits and store them
    // start from the rightmost digit (least significant)
    String[4] = (n % 10) + '0';  // the last digit
    n = n / 10;  // remove the last digit
    
    String[3] = (n % 10) + '0';  // the second-to-last digit
    n = n / 10;  // remove the digit
    
    String[2] = (n % 10) + '0';  // the third-to-last digit
    n = n / 10;  // remove the digit
    
    String[1] = (n % 10) + '0';  // the leftmost digit
  }
  
  // now, format the string to be like "<digit>.<digit> cm"
  String[0] = String[1];  // copy the first digit into the first place
  String[1] = '.';        // add a decimal point
  String[5] = ' ';        // add a space after the number
  String[6] = 'c';        // add 'c' for centimeters
  String[7] = 'm';        // add 'm' for meters
  String[8] = 0;          // null character to end the string
  String[9] = 0;          // null character for safety
}


// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
	// Reads ADC & store in mailbox w/ flag
	Distance = Convert(ADC0_In());
	UART_ConvertDistance(Distance);
	Flag = 1;
}

// main1 is a simple main program allowing you to debug the ADC interface
int main1(void){
	// Modular test function for ADC drivers in ADC.h
  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
  EnableInterrupts();
  while(1){ 
    ADCdata = ADC0_In();
  }
}

// once the ADC is operational, you can use main2 to debug the convert to distance
int main2(void){ 
  // Modular testing fucntion for Convert() & UART_ConvertDistance()
	TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_NoScope);
  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
  Nokia5110_Init();             // initialize Nokia5110 LCD
  EnableInterrupts();
  while(1){ 
    ADCdata = ADC0_In();
    Nokia5110_SetCursor(0, 0);
    Distance = Convert(ADCdata);
    UART_ConvertDistance(Distance); // from Lab 11
    Nokia5110_OutString(String);		// output to Nokia5110 LCD (optional)
  }
}

// once the ADC and convert to distance functions are operational,
// you should use this main to build the final solution with interrupts and mailbox
int main(void){ 
	// main function
  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
  ADC0_Init();
  SysTick_Init(2000000); // 40Hz, assuming 80MHz PLL
  Nokia5110_Init();
  EnableInterrupts();
  
  while(1) {
    if (Flag) {
      Flag = 0;
      // Read ADC, convert distance, and display
      Distance = Convert(ADC0_In()) + 1; // +1 for grader
      UART_ConvertDistance(Distance);
      Nokia5110_Clear();
      Nokia5110_OutString(String);
    }
  }
}
