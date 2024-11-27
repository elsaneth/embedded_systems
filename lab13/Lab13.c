// Lab13.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// edX Lab 13 
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Port B bits 3-0 have the 4-bit DAC
// Port E bits 3-0 have 4 piano keys

#include "..//tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void delay(unsigned long msec);
unsigned long input,previous;

int main(void) { 
  volatile unsigned long retard;
  
  // Real Lab13 
  // for the real board grader to work 
  // you must connect PD3 to your DAC output
  TExaS_Init(SW_PIN_PE3210, DAC_PIN_PB3210, ScopeOn); // activate grader and set system clock to 80 MHz
  SYSCTL_RCGC2_R |= 0x1A; // ports B D F, in main(), you must connect PD3 to your DAC output
  retard = SYSCTL_RCGC2_R;
  
  Sound_Init(); // initialize SysTick timer and DAC
  Piano_Init();
  EnableInterrupts();  // enable after all initialization are done
  previous = Piano_In();

  while(1) {  
    input = Piano_In(); 
    
    // if a key is just pressed (edge detection)
    if (input && (previous == 0)) {
      switch (input) {
				// calculated period parameter for Sound_Tone = System clock frequenzy / (tone frequency x steps per wave)
        case 0x01: Sound_Tone(4778); break; // C (523.251 Hz) PE0 = 80 000 000/(523.251 x 32) 
        case 0x02: Sound_Tone(4257); break; // D (587.330 Hz) PE1
        case 0x04: Sound_Tone(3792); break; // E (659.255 Hz) PE2
        case 0x08: Sound_Tone(3189); break; // G (783.991 Hz) PE3
        default: Sound_Off(); break;
      }
    }
    
    // if a key is held down
    if (input && previous) {
      switch (input) {
        case 0x01: Sound_Tone(4778); break; // C
        case 0x02: Sound_Tone(4257); break; // D
        case 0x04: Sound_Tone(3792); break; // E
        case 0x08: Sound_Tone(3189); break; // G
        default: Sound_Off(); break;
      }
    }

    // ff a key is released
    if (previous && (input == 0)) { 
      Sound_Off(); 
    }
  
    previous = input;
    delay(10); // remove switch bounce    
  }
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
  }
}
