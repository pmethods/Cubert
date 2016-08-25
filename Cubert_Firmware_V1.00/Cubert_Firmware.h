#include <Servo.h>

#define Port Serial
//#define Port Serial1
bool debug = false;          // 0 means dont run robust.  1 means report all actions to Serial port

#define ledpin 13

const int maxVal = 1024; // Open stop point
const int minVal = 0;    // CLosed stop point
const int hystVal = 10;  // acceptable noise - needs to be tuned to catch binding
int valInterval = 200;     // Period for rate of change

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
int brake = 1580;  // tune this to the ESC
int fast = 500;   // adjust to suit - add or subtract this from brake
int slow = 250;   // adjust to suit
int maximum = 2300;  // tune this to the ESC
int minimum = 700;   // tune this to the ESC



// Dead Man Timer that does not use the Delay() function
// Once dead we stay dead and reset LastTime.
// While live we expect that to be...  eh...  less than a minute
long LastTime = 0;           // YOU MUST WATCH OUT FOR OVERFLOW ON THIS...  IT WILL KILL YOU...  
bool Estop = true;            // Should we kill?  (Anyone can set this true including a timeout or any function.  Only timer can clear it.
const int DeadMan = 50;      // mSeconds
long MaxTime = 1000;         // Neck this down perhaps...  we will see.  Stops overflow issues...  stops battery destruction.




// Declarations
byte cmd;  // Stores the next byte of incoming data, which is a "command" to do something 
byte param; // Stores the 2nd byte, which is the command parameter

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
