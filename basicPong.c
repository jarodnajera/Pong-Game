//Basic Pong

//Task Struct
typedef struct task {
   int state;
   unsigned long period;
   unsigned long elapsedTime;
   int (*TickFct)(int);
} task;

//Task Data
task tasks[2];
const unsigned short tasksNum = 2;
const unsigned long tasksPeriodGCD = 1000;
const unsigned long periodBlink = 1000;
const unsigned long periodBall = 1000;

//SM Left Paddle
enum LPADDLE {LPADDLE_START, LPADDLE_wait, LPADDLE_down, LPADDLE_up};
int LPaddle_Tick(int state) {
  //Variables
  unsigned char upBtn = ~PINA; //& to get just that
  unsigned char downBtn = ~PINA; //& to get just that
  static int LPaddlePos = 2;
  //Logic
  switch(state) {
    case LPADDLE_START:
      //set 3 middle leds
      //go to wait btn
      break;
    case LPADDLE_wait:
      if(!upBtn && !downBtn) {
        state = LPADDLE_wait;
      }
      else if(upBtn && i < 3) {
        state = LPADDLE_up;
      }
      else if(downBtn && i > 1) {
        state = LPADDLE_down;
      }
      else {
        state = LPADDLE_wait;
      }
      break;
    case LPADDLE_up:
      if(!upBtn && !downBtn) {
        state = LPADDLE_wait;
      }
      else if(upBtn && i < 3) {
        state = LPADDLE_up;
      }
      else if(downBtn && i > 1) {
        state = LPADDLE_down;
      }
      else {
        state = LPADDLE_wait;
      }
      break;
    case LPADDLE_up:
      if(!upBtn && !downBtn) {
        state = LPADDLE_wait;
      }
      else if(upBtn && i < 3) {
        state = LPADDLE_up;
      }
      else if(downBtn && i > 1) {
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

  //Actions
  switch(state) {

  }
}

//SM Right Paddle (CPU)

//SM Ball
