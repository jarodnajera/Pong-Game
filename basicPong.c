//Basic Pong

#include <xc.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//Task Struct
typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

task tasks[1];
const unsigned short tasksNum = 1;
const unsigned long tasksPeriodGCD = 30;
const unsigned long periodMatrix = 30;


//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
enum Demo_States {shift};
int Demo_Tick(int state) {

	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	static unsigned char row = 0xFE;  	// Row(s) displaying pattern.
	// 0: display pattern on row
	// 1: do NOT display pattern on row
	// Transitions
	switch (state) {
		case shift:
		break;
		default:
		state = shift;
		break;
	}
	// Actions
	switch (state) {
		case shift:
		if (row == 0xEF && pattern == 0x01) { // Reset demo
			pattern = 0x80;
			row = 0xFE;
			} else if (pattern == 0x01) { // Move LED to start of next row
			pattern = 0x80;
			row = (row << 1) | 0x01;
			} else { // Shift LED one spot to the right on current row
			pattern >>= 1;
		}
		break;
		default:
		break;
	}
	PORTC = pattern;	// Pattern to display
	PORTD = row;		// Row(s) displaying pattern
	return state;
}

//Timer Initialization
volatile unsigned char TimerFlag = 0;
void TimerISR() {
	unsigned char i;
	
	for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
	TimerFlag = 1;
}
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet (unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	unsigned char i = 0;
	tasks[i].state = shift;
	tasks[i].period = periodMatrix;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Demo_Tick;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while(1)
    {
		while(!TimerFlag);
		TimerFlag = 0;
    }
	return 0;
}
