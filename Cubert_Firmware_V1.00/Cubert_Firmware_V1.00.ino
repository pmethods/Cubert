/*
 * Cubert control firmware V1.00
 * 
 * Designed by Patrick Schindler for use in Cubert.                            Of course you can use this code anywhere you please....
 * 
 * For use on a MEGA...  as it has the extra hardware serial ports and expanded PWM ports needed.  DO NOT use Software Serial...  tricks are for kids.
 * 
 * Requires Curbert_Firmware.h
 * 
 * Basic Bluetooth communication with Android
 * No security
 * Basic PWM control for actuators using RC ESC:  https://www.amazon.com/gp/product/B00LXCM3Q8/ref=od_aui_detailpages00?ie=UTF8&psc=1
 * Datasheet found here:  http://www.hobbywing.com/goods.php?id=358&filter_attr=0
 * ESC can run FORWARD, REVERSE, BRAKE
 * 
 * 10K pot feedback from custom servos found here:  PA-17-2000   http://www.progressiveautomations.com/heavy-duty-linear-actuator
 * Thats Industrial, 18" travel, 2000lbs, 12V, with 10K pot option added (20 day lead)
 * 
 * Basic Behavior:
 * Init bluetooth, ESC, comms, etc
 * State machine parses command then parameters (which door or doors, what position, how fast)
 *  Dead man timer runs out after 50mS...  forcing user to "keep finger on button" and allowing for a quick stop once in position
 */
#include "Cubert_Firmware.h"
 
// Create doors...  AO pin, AI pin, Max Cal, Min Cal, Last Position, last time, running?
Door bow =   {10, A0, 0, 0, 0, 0, false};
Door stern = {11, A1, 0, 0, 0, 0, false};
Door port =  {12, A2, 0, 0, 0, 0, false};
Door star =  {13, A3, 0, 0, 0, 0, false};


void setup() 
{
  // We init both the Computer COM and the Bluetooth ports...  we dont use #define Port in thise case
  Serial.begin(9600);       // computer
  Serial1.begin(9600);      // Bluetooth
 
  // Assign pin numbers to the PWM output Servos
  bow.side.attach(bow.pwmPin);  
  stern.side.attach(stern.pwmPin);
  port.side.attach(port.pwmPin);
  star.side.attach(star.pwmPin);
  
  // Set all channels to BRAKE - which is the middle of the PWM sweep
  kill();

  pinMode(ledpin, OUTPUT);  // pin 48 onboard LED
  digitalWrite(ledpin, LOW);

  LastTime = millis();

  // Any time we debug we only print to the COM port...  never the Bluetooth...  so we use Serial and not Port
  if(debug) Serial.println("System has been restarted...");
}





void loop() {

  // First we address Estop and the dead man timer.
  if(Estop == true || (millis() - LastTime) > DeadMan)
  {
    kill();
  }

  // Always clear out actionable variables....
  cmd = 0;
  param = 0;

  if ( Port.available() )       // Note that this can be the COM port or the Bluetooth port - change the #define to select which one.  Remember debug mode!!
  {
    cmd = Port.read();
  }
  

  switch ( cmd ) {
      case front:  // Command must be f,b,l,r, or a...  otherwise it flushes.
      case back:
      case left:
      case right:
      case all:
        {
           param = Port.read();
           switch (param)
           {
             case up:
             case down:
             {
                 // Echo back the command and param if in debug mode
                 if(debug) Port.write(cmd);
                 if(debug) Port.write(param);  // CRLF???  May be messy...
                 drive(cmd,param);
               break;
             }  
             default: 
             {
              kill();
              if(debug) Serial.println("param failed parse");
              break;
             }
           } 
        } 
        default: 
        {
          // Kill in any event that does not match.
          kill();
          if(debug) Serial.println("cmd failed parse");
          // normally I would flush the buffer here... but instead we will just kill everything and keep looking.
          break;
        }
    }// end cmd switch

    

  // 50 day rollover protection - only needed very rarely...  dont execute every loop.
  if(LastTime > millis()) LastTime = millis();
}// end loop


/*
 * Stop all motors
 * Set state to not running
 */
void kill()
{
  driveAllPWM(brake);
  if(debug) Serial.println("All Actuators Stopped");
}

void drive(byte cmd, byte param)
{
  int vector = brake + (param == up) ? fast : (-1*fast); // Direction and speed...  brake rests in the middle, so add or subtract magnitude.
  //int limit = 
  
  switch ( cmd ) {
    case front:  
    {
      drivePWM(bow, vector);
    }
    case back:
    {
      drivePWM(stern, vector);
    }
    case left:
    {
      drivePWM(port, vector);
    }
    case right:
    {
      drivePWM(star, vector);
    }
    case all:
    {
        driveAllPWM(vector); 
    }
      default: break;
  }
}


//  Masks the non-portable writeMicroseconds with a function that takes a Servo and a vector
void drivePWM(Door door, int vector)
{
  LastTime = millis();  // reset the dead man timer     // Reset time when and only when we write hardware.
  
  //     not a brake?      Is it at the MAX w request to go further?          or the MIN with request to contract?      or has 200ms gone by without movement?
  if( vector != brake && (door.fbPin >= (maxVal + door.maxCal) && vector > brake ) || (door.fbPin <= (minVal + door.minCal) && vector < brake) || ( (door.lastTime + valInterval > millis()) && (door.fbPin < (door.lastVal + hystVal)) && (door.fbPin > (door.lastVal - hystVal)) ) )
  {
    // Fail - we are at an end point or stalled
    vector = brake;
    if(debug) Serial.println("Command was forced to brake due to limit switch or stall...");
  }

  door.isRunning = (vector == brake)? false : true;
  door.lastVal = door.fbPin;             // grab the current position
  door.side.writeMicroseconds(vector);   // write the new vector (either requested or forced brake)
  door.lastTime = millis();              // timestamp the change
  
}

// Convienience method
void driveAllPWM(int vector)
{
  drivePWM(bow, vector);
  drivePWM(stern, vector);
  drivePWM(port, vector);
  drivePWM(star, vector);
}



// PWM NOTES
// uSeconds....   bow.writeMicroseconds(1500)
// 1000 to 2000 standard
//  700 to 2300 extended
//    1500 centered

// degrees.... bow.write(90)
//  0 one direction max
//  180 opposite direction max
//  90 Cenered
