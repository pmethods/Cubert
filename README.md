# Cubert
Cubert Firmware

UPDATE:  V5 Firmware

ANDROID:  Dropped the bluetooth connect button and implemented auto-connect as well as auto-reconnect.  Constant heartbeat now sent to Arduino to allow Arduino to drop bad Servers.  (Case where a clown connects to our cubert thereby blocking us).  Added specific serial number connection for safety on the first unit...  ONLY works with this specific HC-06 named XYZ.

ARDUINO:  Heartbeat in both directions for auto drop and reset.  No connection issues at all.  

TO DO:  Add service that monitors feedback from actuators to kill power in the event that they are moving while they are not supposed to.  A pair of high reliability 40A relays are inline with servo power and controlled via Arduino to a Mosfet board.  Consider this the primary LVC control as Solar will cover trickle.

TO DO:  Add basic voltage divider to detect LVC @ 10V and cut Servo power.

TO DO LONG TERM:  Add shunt to detect over-current (binding) and run-away situations.  Its better to monitor current than feedback pots as its a direct measure of work being done (I.E. dont care what pots say...  I*V=P



UPDATE: V4 Firmware

10K pot feedback now active and working.
Detects an actuator stall and latches to brake (currently set to 1 second....  200ms probably better)
Note:  The default hist value works but is close - will probably want to open it up depending on rate of change in field
Detects an actuator trying to drive past the top or bottom positions (1V to 4V)
Detects an open or short by runout of the timer...  but aught to do a direct test for under/over!!

Bugs Corrected:  Passing struc by address, analog read on the Ains, logic on the actuator error detection

UPDATE: V3 Firmware

V3 Arduino has had the PWM bugs worked out to the load.  Switch statement corrected.  PWM tuned to specific ESC brand.  Resolution tested.  It was determined that using the brake function resulted in REVERSE running at less than full speed so now running only in F/R mode.  We will see how this turns out.  There is an irritating "feature" to the ESC in that it blipps the output pretty hard at startup...  may be a show-stopper for this low-cost unit.  Been testing with an automotive lightbulb tho - so will have to see.

No 10K pot hooked up so that bit of software obviously isnt working (rolls...  needs to detect open circuit...  and the rate of change should have covered that but isnt...  probably needs more ()()().  Found a possible bug in the compiler around conditionals int X = Y + ? T:F....  required extra ()() to work.

Anyhow - V3 as it stands works (open loop) up to the point of the 10K feedback.  Need to tune that now with external spoof. 
Still need to address Android error on BT popup.


UPDATE:  V2 firmware

V2 Arduino firmware sends feedback to the ANDROID, has field timing (50mS dead man), has the LED indicating when PWM is active, has minor changes and updates

V4 ANDROID software fires button commands on 20mS event, writes the text field with feedback @ 100ms, and throws an irritating error because I am testing BT before it is paired.




The APK file can be installed on any Android phone by downloading it then emailing it to yourself.  Of course...  you must enable that ability on your phone...

The Arduino files can be run on any Mega.  I am using an HC-06.  Power pin is (currently) driven directly off a DIO - chip isnt hot so not a problem (later we will use a mosfet).

At present the Android App allows you to connect via Bluetooth to the Arduino.  Any time you hold down a button that buttons code is sent and the appropriate PWM channel is driven (PWM not tested yet).  A deadman timer on the Arduino slams the pwm to BRAKE after a timeout...  which is equivalent to the response time of a button release (probably about 100ms..  limited by the MIT ANdroid loop time.

TO DO:
Bump the speed up from 9600 to speed up loop times.  Watch out for slope on the pulled-up pins and noise from motors
Confirm that the brushed controllers default to a SAFE, BRAKE, or NEUTRAL state any time they lose PWM
Have the Arduino control a NO relay that powers the lifters...  such that any time the Arduino is DOA the lifters are unpowered
Implement a FAIL SAFE that allows entry into the unit in the case of a bricked or fubar control system

ANDROID
Clean up the button naming
Rework the bluetooth connect
implement some reliability and security
implement change control

ARDUINO
Improve security and reliability
Focus on safety and failing safe
Tune the dead man timer and response time for button lift
Tune the stall detection


Implement change control
