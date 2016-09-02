#include <Servo.h>

//#define Port Serial
#define Port Serial1
bool debug = true;          // 0 means dont run robust.  1 means report all actions to Serial port

#define ledpin 13
#define BT_POWER 12

// Lets call it a sweep from 1V to 4V to give us open and short protection.  That equates to 1024/5= 205 tics per volt
const int maxVal = 819;    //1024; // Open stop point
const int minVal = 205;    // 0; // CLosed stop point
const int hystVal = 10;  // acceptable noise - needs to be tuned to catch binding
int valInterval = 1000;  //200;     // Period for rate of change

struct Door {
  int pwmPin; // PWM output pin
  int fbPin;  // Analog input pin
  int maxCal; // Cal for open
  int minCal; // cal for closed
  int lastVal; // last known position
  long lastTime; // last time stamp
  bool isRunning;  // are we running?
  Servo side;    // PWM object
};


// State Machine
const int ESTOP = 0;
const int LISTEN = 1;
const int EXECUTE = 2;
int state = ESTOP;


// PWM values in uSeconds to control RC ESC with FWD, REV, BRAKE
int brake = 1500;  //1580;  // tune this to the ESC
int fast = 500;  //500;   // adjust to suit - add or subtract this from brake
int slow = 250;   // adjust to suit
int maximum = 2300;  // tune this to the ESC
int minimum = 700;   // tune this to the ESC

// 2290 max if coming from neutral.  YOu can creep past 2000 tho
// >1800 full blast
// <1100 reverse full blast
// 1480 to 1515 dead band ish
// Switching pin to input kills ESC output - no need to attach or detach servo library


// Dead Man Timer that does not use the Delay() function
// Once dead we stay dead and reset LastTime.
// While live we expect that to be...  eh...  less than a minute
long LastTime = 0;           // YOU MUST WATCH OUT FOR OVERFLOW ON THIS...  IT WILL KILL YOU...  
bool Estop = false;            // Should we kill?  (Anyone can set this true including a timeout or any function.  Only timer can clear it.
const int DeadMan = 100;      // mSeconds
long MaxTime = 1000;         // Neck this down perhaps...  we will see.  Stops overflow issues...  stops battery destruction.




// Declarations
char cmd;  // Stores the next byte of incoming data, which is a "command" to do something 
char param; // Stores the 2nd byte, which is the command parameter
bool report = false;
bool BTupdate = false;
// Command list which, what, who
/*
 * Example:
 * fue = front up fast
 * fuc = front up slow
 * aue = all up fast
 * fde = front down fast
 * ade = all down fast
 * luc = left up slow
 * etc...
 * 
 * Any command other than these results in a parase error.  Buffer is cleared, motors are stopped, state machine is reset.
 */
 
// who - determins how agressive the system is.  Engineers can run it fast and hard.  Customers get a tamed down version.
//static byte engineer = 'e';
//static byte customer = 'c';

// which
const byte front = 'f';
const byte back = 'b';
const byte left = 'l';
const byte right = 'r';
const byte all = 'a';

// what
const byte up = 'u';
const byte down = 'd';
