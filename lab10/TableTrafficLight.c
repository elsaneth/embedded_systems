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
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014)) // set to the number of bus cycles one wishes to wait
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018)) // will clear the counter and will clear the count flag (bit 16) of the CTRL register
#define TRAFFIC_LIGHTS			(*((volatile unsigned long *)0x400050FC))
#define PEDESTRIAN_LIGHTS		(*((volatile unsigned long *)0x40025028))
#define SENSORS							(*((volatile unsigned long *)0x4002401C))
	
#define goWest 0
#define waitWest 1
#define goSouth 2
#define waitSouth 3
#define walk 4
#define notWalk1 5
#define walkOff 6 // flashing red
#define notWalk2 7

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
void ports_Init(void){
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x32; // activate clock for Port B,E,F
  delay = SYSCTL_RCGC2_R; // allow time for clock to start
	
	// Port B
	GPIO_PORTB_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTB_CR_R = 0x3F; // allow changes to PB5-0
  GPIO_PORTB_DEN_R |= 0x3F; // enable digital I/O on PB5-0
	GPIO_PORTB_DIR_R |= 0x3F; // PB5-0 outputs
	
	// Port E
	GPIO_PORTE_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTE_CR_R = 0x07; // allow changes to PE2-0
  GPIO_PORTE_PUR_R &= ~0x07; // disableb pull-up on PE2-0
  GPIO_PORTE_DEN_R |= 0x07; // enable digital I/O on PE2-0
	GPIO_PORTE_DIR_R &= ~0x07; // PE2-0 inputs

	// Port F
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTF_CR_R = 0x0A; // allow changes to PF1 & PF3
  GPIO_PORTF_DEN_R |= 0x0A; // enable digital I/O on PF1 & PF3
	GPIO_PORTF_DIR_R |= 0x0A; // PF1 & PF3 outputs
}

//chapter 10
void SysTick_Init(void) {
    NVIC_ST_CTRL_R = 0; // disable SysTick during setup
    NVIC_ST_CTRL_R = 0x05; //eEnable SysTick with core clock
}

//chapter 10
void SysTick_Wait(uint32_t delay){
  NVIC_ST_RELOAD_R = delay-1; // number of counts to wait
  NVIC_ST_CURRENT_R = 0; // any value written to CURRENT clears
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

typedef struct State {
	uint32_t TrafficLightoutput;
	uint32_t PedestrianLightOutput;
	uint32_t StateDuration;
	uint32_t NextState[8];
} State;

int main(void){ 
	uint32_t S = 0; // current state
	State FSM[9] = {
    // State: goWest
    {0x0C, 0x02, 10, {goWest, goWest, waitWest, waitWest, waitWest, waitWest, waitWest, waitWest}},

    // State: waitWest
    {0x14, 0x02, 10, {goWest, goWest, goSouth, goSouth, walk, walk, goSouth, goSouth}},

    // State: goSouth
    {0x21, 0x02, 10, {goSouth, waitSouth, goSouth, waitSouth, waitSouth, waitSouth, waitSouth, waitSouth}},

    // State: waitSouth
    {0x22, 0x02, 10, {goSouth, goWest, goSouth, goWest, walk, walk, walk, walk}},

    // State: walk
    {0x24, 0x08, 10, {walk, notWalk1, notWalk1, notWalk1, walk, notWalk1, notWalk1, notWalk1}},

    // State: notWalk
    {0x24, 0x02, 10, {notWalk1, walkOff, walkOff, walkOff, walkOff, walkOff, walkOff, walkOff}},

    // State: walkOff
    {0x24, 0x00, 10, {walkOff, notWalk2, notWalk2, notWalk2, notWalk2, notWalk2, notWalk2, notWalk2}}
	};
	
	// Initialisation
	TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210, ScopeOn);
	ports_Init(); // initialise ports
	SysTick_Init(); // initialise systick
	
	EnableInterrupts();
  while(1){
    TRAFFIC_LIGHTS = FSM[S].TrafficLightoutput;  // set traffic lights
		PEDESTRIAN_LIGHTS = FSM[S].PedestrianLightOutput; // set pedestrian lights
    SysTick_Wait10ms(FSM[S].StateDuration);// delay
    S = FSM[S].NextState[SENSORS];  // next state
  }
}
