# Cubert
Cubert Firmware

UPDATE:  

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
