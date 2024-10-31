// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0

// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)

// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

// ***** 2. Global Declarations Section *****
#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))
#define TRAFFIC_LIGHTS			(*((volatile unsigned long *)0x400050FC))
#define PEDESTRIAN_LIGHTS		(*((volatile unsigned long *)0x40025028))
#define SENSORS							(*((volatile unsigned long *)0x4002401C))
	
#define goWest 0
#define waitWest 1
#define goSouth 2
#define waitSouth 3
#define walk 4
#define notWalk1 5
#define walkOff1 6
#define notWalk2 7
#define walkOff2 8

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
void ports_Init(void){
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x32; // activate clock for Port B,E,F
  delay = SYSCTL_RCGC2_R; // allow time for clock to start
	
	// Port B
  GPIO_PORTB_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5-0
	GPIO_PORTB_PCTL_R = 0x00000000;   // clear PCTL
  GPIO_PORTB_AMSEL_R &= ~0x3F;      // disable analog on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F;      // disable alt funct on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;         // enable digital I/O on PB5-0
	GPIO_PORTB_DIR_R |= 0x3F;         // PB5-0 outputs
	
	// Port E
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTE_CR_R = 0x07;           // allow changes to PE2-0
	GPIO_PORTE_PCTL_R = 0x00000000;   // clear PCTL
  GPIO_PORTE_AMSEL_R &= ~0x07;      // disable analog on PE2-0
  GPIO_PORTE_AFSEL_R &= ~0x07;      // disable alt funct on PE2-0
  GPIO_PORTE_PUR_R &= ~0x07;        // disableb pull-up on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;         // enable digital I/O on PE2-0
	GPIO_PORTE_DIR_R &= ~0x07;        // PE2-0 inputs

	// Port F
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTF_CR_R = 0x0A;           // allow changes to PF1 & PF3
	GPIO_PORTF_PCTL_R = 0x00000000;   // clear PCTL
  GPIO_PORTF_AMSEL_R &= ~0x0A;      // disable analog on PF1 & PF3
  GPIO_PORTF_AFSEL_R &= ~0x0A;      // disable alt funct on PF1 & PF3
  GPIO_PORTF_DEN_R |= 0x0A;         // enable digital I/O on PF1 & PF3
	GPIO_PORTF_DIR_R |= 0x0A;         // PF1 & PF3 outputs
}

//chapter 10
void SysTick_Init(void) {
    NVIC_ST_CTRL_R = 0; // Disable SysTick during setup
    NVIC_ST_CTRL_R = 0x05; // Enable SysTick with core clock
}

//chapter 10
void SysTick_Wait(uint32_t delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}

// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(uint32_t delay){
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

typedef struct Stype{
	uint32_t TrafficLightoutput;
	uint32_t PedestrianLightOutput;
	uint32_t StateDuration;
	uint32_t NextState[8];
} SType;

int main(void){ 
	uint32_t S = 0; // current state
	SType FSM[9] = {
	{0x0C, 0x02, 10, {goWest, goWest, waitWest, waitWest, waitWest, waitWest, waitWest, waitWest}},					// goWest
	{0x14, 0x02, 10, {goWest, goWest, goSouth, goSouth, walk, walk, goSouth, goSouth}},											// waitWest
	{0x21, 0x02, 10, {goSouth, waitSouth, goSouth, waitSouth, waitSouth, waitSouth, waitSouth, waitSouth}},	//goSouth
	{0x22, 0x02, 10, {goSouth, goWest, goSouth, goWest, walk, walk, walk, walk}},														// waitSouth
	{0x24, 0x08, 10, {walk, notWalk1, notWalk1, notWalk1, walk, notWalk1, notWalk1, notWalk1}},							// walk
	{0x24, 0x02, 10, {notWalk1, walkOff1, walkOff1, walkOff1, walkOff1, walkOff1, walkOff1, walkOff1}},			// notWalk1
	{0x24, 0x00, 10, {walkOff1, notWalk2, notWalk2, notWalk2, notWalk2, notWalk2, notWalk2, notWalk2}},			// walkOff1
	{0x24, 0x02, 10, {notWalk2, walkOff2, walkOff2, walkOff2, walkOff2, walkOff2, walkOff2, walkOff2}},			//notWalk2
	{0x24, 0x00, 10, {walkOff2, goWest, goSouth, goWest, walk, goWest, goSouth, goWest}}};
	
	// Initialisation
	TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210, ScopeOn);
	ports_Init(); // Initialise port B,E,F
	SysTick_Init(); // Initialise systick
  EnableInterrupts();
	
	// Looping through FSM
  while(1){
    TRAFFIC_LIGHTS = FSM[S].TrafficLightoutput;  // set traffic lights
		PEDESTRIAN_LIGHTS = FSM[S].PedestrianLightOutput; // set pedestrian lights
    SysTick_Wait10ms(FSM[S].StateDuration);// delay
    S = FSM[S].NextState[SENSORS];  // next state
  }
}
