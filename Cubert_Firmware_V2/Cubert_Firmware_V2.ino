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
Door bow =   {2, A0, 0, 0, 0, 0, false};
Door stern = {3, A1, 0, 0, 0, 0, false};
Door port =  {4, A2, 0, 0, 0, 0, false};
Door star =  {5, A3, 0, 0, 0, 0, false};


void setup() 
{
  // We init both the Computer COM and the Bluetooth ports...  we dont use #define Port in thise case
  Serial.begin(115200);       // computer
  Serial1.begin(57600,SERIAL_8N1);      // Bluetooth  Cubert001 is the name, 0001 is the passcode - changed per AT commands listed at bottom
  // Note:  Do not exceed 9,600 unless required.  20K pullup enabled... slew will kill comms unless done better.

  pinMode(19,INPUT_PULLUP);
 
  // Assign pin numbers to the PWM output Servos
  bow.side.attach(bow.pwmPin);  
  stern.side.attach(stern.pwmPin);
  port.side.attach(port.pwmPin);
  star.side.attach(star.pwmPin);
  
  // Set all channels to BRAKE - which is the middle of the PWM sweep
  kill(true);

  pinMode(ledpin, OUTPUT);  // pin 48 onboard LED
  digitalWrite(ledpin, LOW);

  pinMode(BT_POWER, OUTPUT);  // pin 48 onboard LED
  digitalWrite(BT_POWER, HIGH);

  LastTime = millis();

  // Any time we debug we only print to the COM port...  never the Bluetooth...  so we use Serial and not Port
  if(debug) Serial.println("System has been restarted...");
  if(debug) Port.println("System has been restarted...");

  BTupdate = true;
  
  
}





void loop() {

  // First we address Estop and the dead man timer.
  if(Estop == true || (millis() - LastTime) > DeadMan)
  {
    kill(false);
    if(debug) Serial.println("Deadman Tripped...");
    LastTime = millis();
    // hmmm...  everytime we hit the brakes (on every loop...) it resets.
    // debug only
    digitalWrite(ledpin, LOW);
    if(!BTupdate) Port.println("System Brake ON");
    BTupdate = true;
  }

  // Always clear out actionable variables....
  cmd = 0;
  param = 0;

  if ( Port.available() > 1)       // Note that this can be the COM port or the Bluetooth port - change the #define to select which one.  Remember debug mode!!
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
                 if(debug) Serial.print("Successful Parse - ");
                 if(debug) Serial.write(cmd);
                 if(debug) Serial.write(param);  // CRLF???  May be messy...
                 if(debug) Serial.println("");
                 if(BTupdate) Port.print("Driving ");
                 if(BTupdate) Port.write(cmd);
                 if(BTupdate) Port.write(param);  // CRLF???  May be messy...
                 if(BTupdate) Port.println(" door");
                 BTupdate = false;
                 digitalWrite(ledpin, HIGH);
                 drive(cmd,param);
               break;
             }  // end up/down
             default:  if(debug) Serial.println("param failed parse");
           }  // end param 
           break;  // break for good cmd
        } // end all
        case 0:  break; 
        default: if(debug) Serial.println("cmd failed parse");
    }// end cmd switch

    

  // 50 day rollover protection - only needed very rarely...  dont execute every loop.
  //if(LastTime > millis()) LastTime = millis();
}// end loop


/*
 * Stop all motors
 * Set state to not running
 */
void kill(bool report)
{
  driveAllPWM(brake);
  if(debug && report) Serial.println("All Actuators Stopped");
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
  if(vector != brake) 
  {
    LastTime = millis();  // reset the dead man timer     // Reset time when and only when we write hardware.
    digitalWrite(ledpin, HIGH);
  }
  else digitalWrite(ledpin, false);
  
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


/*
 *   Cubert001, 0001 is password - password follows cubert, name revs
Command Response  Comment
AT  OK  Does nothing!
AT+VERSION  OKlinvorV1.5  The firmware version
AT+NAMExyz  OKsetname Sets the module name to "xyz"
AT+PIN1234  OKsetPIN  Sets the module PIN to 1234
AT+BAUD1  OK1200  Sets the baud rate to 1200
AT+BAUD2  OK2400  Sets the baud rate to 2400
AT+BAUD3  OK4800  Sets the baud rate to 4800
AT+BAUD4  OK9600  Sets the baud rate to 9600
AT+BAUD5  OK19200 Sets the baud rate to 19200
AT+BAUD6  OK38400 Sets the baud rate to 38400
AT+BAUD7  OK57600 Sets the baud rate to 57600
AT+BAUD8  OK115200  Sets the baud rate to 115200
AT+BAUD9  OK230400  Sets the baud rate to 230400
AT+BAUDA  OK460800  Sets the baud rate to 460800
AT+BAUDB  OK921600  Sets the baud rate to 921600
AT+BAUDC  OK1382400 Sets the baud rate to 138240



 */
