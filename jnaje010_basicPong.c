//Basic Pong

/* Jarod Najera jnaje010@ucr.edu
 * Section 022
 * Lab 11 Advancement 0
 * Basic Pong
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work
 */


#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//GetBit
unsigned char GetBit(unsigned char x, unsigned char k) {
	return((x & (0x01 << k)) != 0);
}

//Find GCD
unsigned long int findGCD(unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while(1) {
		c = a % b;
		if(c == 0) { return b; }
		a = b;
		b = c;
	}
	return 0;
}

//Task Struct
typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

//Game Variables
//Reset
unsigned char reset = 0x00;
//Game Mode
unsigned char gameMode = 0x00;
//P1 Score
unsigned char lScore = 0x00;
//P2 Score
unsigned char rScore = 0x00;
//Y-Bounds

//Paddle Variables
//Left Paddle Positions
//Top, Mid, Bottom
unsigned char lPaddleTop[3] = {0xFE, 0xFD, 0xFB};
unsigned char lPaddleMid[3] = {0xFD, 0xFB, 0xF7};
unsigned char lPaddleBot[3] = {0xFB, 0xF7, 0xEF};

int lPaddlePos = 1;
//unsigned char lPaddleDisplay = 0x00;

//Right Paddle Positions
//Top, Mid, Bottom
unsigned char rPaddle[3] = {0x18, 0x11, 0x03};
int rPaddlePos = 1;

//CPU Paddle Positions
//Top, Mid, Bottom
unsigned char CPUPaddleTop[3] = {0xFE, 0xFD, 0xFB};
unsigned char CPUPaddleMid[3] = {0xFD, 0xFB, 0xF7};
unsigned char CPUPaddleBot[3] = {0xFB, 0xF7, 0xEF};

int CPUPaddlePos = 1;
//unsigned char CPUPaddleDisplay = 0x00;

//Ball Variables
//Ball Position
unsigned char ballPosX = 0x20;
unsigned char ballPosY = 0x1B;
unsigned char ballUp = 0x00;
unsigned char ballDown = 0x01;
unsigned long ballSpeed = 150;


//Check for paddle Y at 0100 0000
//Ball Movement ============================================================
enum BALL {BALL_START, BALL_right, BALL_left, BALL_reset};
int BallTick(int state) {
	switch(state) {
		case BALL_START:
			state = BALL_right;
			break;
		case BALL_right:
			ballPosX >>= 1;
			if(ballDown) {
				ballPosY = (ballPosY << 1) | 0x01;
			}
			if(ballUp) {
				ballPosY = ((ballPosY | 0x80) >> 1) | 0x80;
			}
			if(GetBit(ballPosY, 4) == 0) {
				ballDown = 0x00;
				ballUp = 0x01;
			}
			if(GetBit(ballPosY, 0) == 0) {
				ballDown = 0x01;
				ballUp = 0x00;
			}

			if(ballPosX == 0x01 && (ballPosY == (CPUPaddleMid[CPUPaddlePos]))) {
				state = BALL_left;
			}
			else if(ballPosX == 0x01 && !(ballPosY == (CPUPaddleMid[CPUPaddlePos])))		{
				state = BALL_reset;
			}
			else {
				state = BALL_right;
			}
			break;
		case BALL_left: 
		        ballPosX <<= 1;
                        if(ballDown) {
                                ballPosY = (ballPosY << 1) | 0x01;
                        }
                        if(ballUp) {
                                ballPosY = ((ballPosY | 0x80) >> 1) | 0x80;
                        }
                        if(GetBit(ballPosY, 4) == 0) {
                                ballDown = 0x00;
                                ballUp = 0x01;
                        }
                        if(GetBit(ballPosY, 0) == 0) {
                                ballDown = 0x01;
                                ballUp = 0x00;
                        }

                        if(ballPosX == 0x80 && (ballPosY == (lPaddleMid[lPaddlePos]))) {
                                state = BALL_right;
                        }
                        else if(ballPosX == 0x80 && !(ballPosY == (lPaddleMid[lPaddlePos])))                      {
                                state = BALL_reset;
                        }
                        else {
                                state = BALL_left;
                        }
                        break;
		case BALL_reset:
			ballPosX = 0x20;
			ballPosY = 0x1B;
			ballUp = 0x00;
			ballDown = 0x01;
			state = BALL_right;
			break;
		default:
			state = BALL_START;
			break;
	}


return state;
}

//Reset Tick =============================================================
enum SOFTRESET {SOFTRESET_START, SOFTRESET_waitBtn, SOFTRESET_reset};
int SoftResetTick(int state) {
	unsigned char resetBtn = ~PINA & 0x04;
	switch(state) {
		case SOFTRESET_START:
			state = SOFTRESET_waitBtn;
			break;
		case SOFTRESET_waitBtn:
			if(resetBtn) {
				PORTC = 0x00;
                                PORTD = 0x00;
				ballPosX = 0x20;
                        	ballPosY = 0x1B;
                       	 	ballUp = 0x00;
                        	ballDown = 0x01;
				state = SOFTRESET_reset;
			}
			else {
				state = SOFTRESET_waitBtn;
			}
			break;
		case SOFTRESET_reset:
			if(resetBtn) {
				PORTC = 0x00;
                                PORTD = 0x00;
				ballPosX = 0x20;
                        	ballPosY = 0x1B;
                        	ballUp = 0x00;
                       		 ballDown = 0x01;
				state = SOFTRESET_reset;
			}
			else {
				state = SOFTRESET_waitBtn;
			}
			break;
		default:
			state = SOFTRESET_START;
	}
	return state;
}


//Left Paddle SM (Player 1) =================================================
enum LPADDLE {LPADDLE_START, LPADDLE_wait, LPADDLE_down, LPADDLE_up};
int LPaddleTick(int state) {
	unsigned char upBtn = ~PINA & 0x01;
	unsigned char dwnBtn = ~PINA & 0x02;

	switch(state) {
		case LPADDLE_START:
			state = LPADDLE_wait;
			break;
		case LPADDLE_wait:
			if(!upBtn && !dwnBtn) {
				state = LPADDLE_wait;
			}
			else if(upBtn && (lPaddlePos < 2)) {
				state = LPADDLE_up;
			}
			else if(dwnBtn && (lPaddlePos > 0)) {
				state = LPADDLE_down;
			}
			else {
				state = LPADDLE_wait;
			}
			break;
		case LPADDLE_down:
			if(!upBtn && !dwnBtn) {
				state = LPADDLE_wait;
			}
			else if(upBtn && (lPaddlePos < 2)) {
				state = LPADDLE_up;
			}
			else if(dwnBtn && (lPaddlePos > 0)) {
				state = LPADDLE_down;
			}
			else {
				state = LPADDLE_wait;
			}
			break;
		case LPADDLE_up:
			if(!upBtn && !dwnBtn) {
                                state = LPADDLE_wait;
                        }
                        else if(upBtn && (lPaddlePos < 2)) {
                                state = LPADDLE_up;
                        }
                        else if(dwnBtn && (lPaddlePos > 0)) {
                                state = LPADDLE_down;
                        }
                        else {
                                state = LPADDLE_wait;
                        }
                        break;
		default:
			state = LPADDLE_wait;
			break;
	}

	switch(state) {
		case LPADDLE_wait:
			//lPaddleDisplay = lPaddle[lPaddlePos];
			break;
		case LPADDLE_up:
			lPaddlePos = lPaddlePos + 1;
			//lPaddleDisplay = lPaddle[lPaddlePos];
			break;
		case LPADDLE_down:
			lPaddlePos = lPaddlePos - 1;
			//lPaddleDisplay = lPaddle[lPaddlePos];
			break;
		default:
			break;
	}
	return state;
}

//Right Paddle SM (CPU) ===============================================
enum CPUPADDLE {CPUPADDLE_START, CPUPADDLE_wait, CPUPADDLE_down, CPUPADDLE_up};
int CPUPaddleTick(int state) {
	unsigned char CPUMove = 0x00;

        switch(state) {
                case CPUPADDLE_START:
                        state = CPUPADDLE_wait;
                        break;
                case CPUPADDLE_wait:
			CPUMove = rand() % 3;
                        if(CPUMove == 1) {
                                state = CPUPADDLE_wait;
                        }
                        else if(CPUMove == 2 && (CPUPaddlePos < 2)) {
                                state = CPUPADDLE_up;
                        }
                        else if(CPUMove == 0 && (CPUPaddlePos > 0)) {
                                state = CPUPADDLE_down;
                        }
                        else {
                                state = CPUPADDLE_wait;
                        }
                        break;
		case CPUPADDLE_down:
                  	CPUMove = rand() % 3;
                        if(CPUMove == 1) {
                                state = CPUPADDLE_wait;
                        }
                        else if(CPUMove == 2 && (CPUPaddlePos < 2)) {
                                state = CPUPADDLE_up;
                        }
                        else if(CPUMove == 0 && (CPUPaddlePos > 0)) {
                                state = CPUPADDLE_down;
                        }
                        else {
                                state = CPUPADDLE_wait;
                        }
                        break;
                case CPUPADDLE_up:
                        CPUMove = rand() % 3;
                        if(CPUMove == 1) {
                                state = CPUPADDLE_wait;
                        }
                        else if(CPUMove == 2 && (CPUPaddlePos < 2)) {
                                state = CPUPADDLE_up;
                        }
                        else if(CPUMove == 0 && (CPUPaddlePos > 0)) {
                                state = CPUPADDLE_down;
                        }
                        else {
                                state = CPUPADDLE_wait;
                        }
                        break;
		default:
			state = CPUPADDLE_START;
			break;
	}

	switch(state) {
                case CPUPADDLE_wait:
                       //CPUPaddleDisplay = CPUPaddle[CPUPaddlePos];
                        break;
                case CPUPADDLE_up:
                        CPUPaddlePos = CPUPaddlePos + 1;
                       // CPUPaddleDisplay = CPUPaddle[CPUPaddlePos];
                        break;
                case CPUPADDLE_down:
                        CPUPaddlePos = CPUPaddlePos - 1;
                        //CPUPaddleDisplay = CPUPaddle[CPUPaddlePos];
                        break;
                default:
                        break;
        }
        return state;
}

//Display Tick =============================================================
enum DISPLAY {DISPLAY_START, DISPLAY_lPaddle, DISPLAY_CPUPaddle, DISPLAY_ball};
int DisplayTick(int state) {
	switch(state) {
		case DISPLAY_START:
			state = DISPLAY_lPaddle;
			break;
		case DISPLAY_lPaddle:
			state = DISPLAY_CPUPaddle;
			break;
		case DISPLAY_CPUPaddle:
			state = DISPLAY_ball;
			break;
		case DISPLAY_ball:
			state = DISPLAY_lPaddle;
		default:
			state = DISPLAY_START;
			break;
	}
	
	switch(state) {
		case DISPLAY_lPaddle:
			PORTD = lPaddleTop[lPaddlePos] & lPaddleMid[lPaddlePos] & lPaddleBot[lPaddlePos];
			PORTC = 0x80;
			break;
		case DISPLAY_CPUPaddle:
			PORTD = CPUPaddleTop[CPUPaddlePos] & CPUPaddleMid[CPUPaddlePos] & CPUPaddleBot[CPUPaddlePos];
			PORTC = 0x01;
			break;
		case DISPLAY_ball:
			PORTD = ballPosY;
			PORTC = ballPosX;
			break;
		default:
			break;
	}

	return state;
}

//Timer Initialization ====================================================
volatile unsigned char TimerFlag = 0;
void TimerISR(){
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

// MAIN ==============================================
int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRA = 0x00; PORTA = 0xFF;

	static task taskDis, taskLPad, taskCPUPad, taskBall, taskReset;
	task *tasks[] = {&taskDis, &taskLPad, &taskCPUPad, &taskBall, &taskReset};
	const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

	taskDis.state = DISPLAY_START;
	taskDis.period = 5;
	taskDis.elapsedTime = taskDis.period;
	taskDis.TickFct = &DisplayTick;

	taskLPad.state = LPADDLE_START;
	taskLPad.period = 100;
	taskLPad.elapsedTime = taskLPad.period;
	taskLPad.TickFct = &LPaddleTick;

	taskCPUPad.state = CPUPADDLE_START;
	taskCPUPad.period = 150;
	taskCPUPad.elapsedTime = taskCPUPad.period;
	taskCPUPad.TickFct = &CPUPaddleTick;

	taskBall.state = BALL_START;
	taskBall.period = 150;
	taskBall.elapsedTime = taskBall.period;
	taskBall.TickFct = &BallTick;

	taskReset.state = SOFTRESET_START;
	taskReset.period = 5;
	taskReset.elapsedTime = taskReset.period;
	taskReset.TickFct = &SoftResetTick;

	unsigned long GCD = tasks[0]->period;
	unsigned short i;
	for(i = 1; i < numTasks; i++) {
        	GCD = findGCD(GCD, tasks[i]->period);
	}	

	TimerSet(GCD);
	TimerOn();
	
    while(1)
    {
	    for(i = 0; i < numTasks; i++) {
		    if(tasks[i]->elapsedTime >= tasks[i]->period) {
			 tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			 tasks[i]->elapsedTime = 0;
			}
		    tasks[i]->elapsedTime += GCD;
		}
	  
		while(!TimerFlag);
		TimerFlag = 0;
    }
	return 0;
}
