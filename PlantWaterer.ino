// PlantWaterer
// by Nathan Gardner <nathan@factory8.com>
// This checks the moisture of soil, and if it is dry, opens up a water valve until its wet.

#include <Servo.h>
float versionNumber = 0.02;
String versionDate = " 03/01/2014";

// ---------------
// pin definitions
// ---------------
// analog pins
const int SM1sensorPin = 0; // soil moisture 1
// digital pins
const int SM1powerPin = 6; // used to power the sensor intermitantly
const int WV1controlPin = 9; // used to control the WV1
const int ledPin = 13; // notification LED

// -------------------------
// soil moisture (SM)
// -------------------------
const float SM1sensorTuning = 2.8; // how much you need to multiply by to get 1023 in full wet, should be user adjustable
int SM1wetLevel = 150; // 0-1023, when is it considered wet, should be user adjustable
const unsigned long SMsensorTimer_powerOn = 100; // how long to let power on take
const unsigned long SMsensorTimer_read = 10; // how long to read the input
const unsigned long SMsensorTimer_powerOff = 100; // how long to let power down
const unsigned long SMsensorTimer_sleep = 5000; // how long to wait between checks
int SMprocessStep, SM1sensorValue, SM1sensorValueRaw, SM1moistureLevel, SM1isWet = 0;
unsigned long SMsensorTimerRateCurrent,SMtimerLast, SMtimerNext, SMsensorCycles = 0;
float SM1moisturePercent = 0;

// ---------------------------
// water valve (WV) - this is a servo
// ---------------------------
Servo WV1servo; // the water valve servo
int WV1position = 0; // position of water valve
int WV1positionOpen = 180; // position when open
int WV1positionClosed = 0; // position when closed
int WVprocessStep = 0;
const unsigned long WV1timer_watering = 10000; // how long to water for
const unsigned long WV1timer_soak = 5000; // how long to let water soak into soil
unsigned long WVtimerNext, WVtimerRateCurrent, WVwateringCyles, WVtimerLast = 0;

// ---------------------------
// prcoess variables
// ---------------------------
unsigned long timerCurrentTime; // global timer
int processSwitchTime = 500; // how long to wait to swtich between processes
int numbFlashes, flashNumb = 0; // for notificaiton led
// different processe
const int processStartup = 0;
const int processSoilCheck = 1;
const int processWatering = 2;
int runningProcess = processStartup;

// ---------------------------
// Arduino standard setup()
// ---------------------------

void setup(){

	//delay(1000); //allow lcd to wake up.
	
	// analog pins
	pinMode(SM1sensorPin,INPUT);
	
	// digital pins
	pinMode(SM1powerPin,OUTPUT);
        WV1servo.attach(WV1controlPin); // connect to WV1
	
	// setup serial communication
	Serial.begin(9600);

}

// --------------------------
// >> Arduino standard loop()
// --------------------------

void loop(){

	// for all timers
	timerCurrentTime = millis();
        
        // what process are we running
        switch(runningProcess) {
          case processStartup:
            STARTprocess();
            runningProcess = processSoilCheck;
          break;
          case processSoilCheck:
            SMprocess();
          break;
          case processWatering:
            WVprocess();
          break;
        }
}

void STARTprocess() {
  Serial.println("PlantWaterer, by Nathan Gardner");
  Serial.print("version "); Serial.print(versionNumber); Serial.print(" "); Serial.print(versionDate);
  Serial.println("-------------------------------");
  
  // turn off water by default
  WV1servo.write(WV1positionClosed);
  
  // turn off notificaton led
  digitalWrite(ledPin,LOW);
  
  delay(1000);
}

// ---------------------------
// SM functions
// ---------------------------
// reset them when the process gets unpaused
void resetSMtimers() {
  SMtimerNext = timerCurrentTime+processSwitchTime;
}

void updateSM(){
    
    SM1sensorValueRaw = analogRead(SM1sensorPin); // invert reading
    SM1sensorValue = SM1sensorValueRaw * SM1sensorTuning;
    SM1moistureLevel = constrain(SM1sensorValue,0,1023);
    SM1moisturePercent = ((float)SM1moistureLevel/1023)*100;
    
}

void SMprocess() {
  // -------------------------------------------
  // slow timer for sensors (stepped processing timer)
  // -------------------------------------------
  if ( timerCurrentTime >= SMtimerNext) {
  
                // last time it ran
	        SMtimerLast = timerCurrentTime;

		// sensor process stepping
		switch(SMprocessStep){
		case 0: // init
                  Serial.println("== starting soil moisture process ==");
		  break;
		case 1: // powerOn 
		  SMsensorTimerRateCurrent = SMsensorTimer_powerOn;
                  Serial.println("== powering on soil moisture ==");
		  digitalWrite(SM1powerPin,HIGH);
		  break;
		case 2: // read 
		  SMsensorTimerRateCurrent = SMsensorTimer_read;
                  Serial.println("== reading soil moisture ==");
		  updateSM();
		  break;
		case 3: // powerOff
		  SMsensorTimerRateCurrent = SMsensorTimer_powerOff;
                  Serial.println("== powering off soil moisture ==");
		  digitalWrite(SM1powerPin,LOW);
		  break;
		case 4: // check results, and take action

                  Serial.println("== checking soil moisture results ==");
                  
                  reportLevels();
                  
                  // ground is wet!
                  if(SM1moistureLevel >= SM1wetLevel) {
                   Serial.println("== soil is wet ==");
                   SM1isWet = 1;
                   SMsensorTimerRateCurrent = SMsensorTimer_sleep;
                  } else {
                   // ground is dry!
                   Serial.println("== soil is dry ==");
                   SM1isWet = 0;
                   Serial.println("== switching to watering process ==");
                   runningProcess = processWatering;
                   resetWVtimers();
                  }
                  
		  break;
		}

                // max 5 steps
                if(SMprocessStep == 4) { SMsensorCycles++; SMprocessStep = 1; } else { SMprocessStep++; }
                
                SMtimerNext = SMtimerLast + SMsensorTimerRateCurrent;
  }
}

// ---------------------------
// servo functions
// ---------------------------
void resetWVtimers() {
  WVtimerNext = timerCurrentTime+processSwitchTime;
}

void WVprocess() {
  
  // turn on notification
  digitalWrite(ledPin,HIGH);
  
  if ( timerCurrentTime >= WVtimerNext) {
    
    WVtimerLast = timerCurrentTime;
    
    switch(WVprocessStep) {
      case 0: // init
        Serial.println("== starting watering process ==");
        WVtimerRateCurrent = 0;
      break;
      case 1: // watering
        if(SM1isWet == 0) {
          Serial.println("== watering WV1 ==");
          WV1servo.write(WV1positionOpen); // open
          delay(15);
        }
        WVtimerRateCurrent = WV1timer_watering;
      break;
      
      case 2: // stop watering
        Serial.println("== closing watering valves, soaking ==");
        WV1servo.write(WV1positionClosed); // close
        delay(15);
        WVtimerRateCurrent = WV1timer_soak;
      break;
      
      case 3: // switch processes
        Serial.println("== switching to soil moisture process ==");
        // turn off notification
        digitalWrite(ledPin,LOW);
        runningProcess = processSoilCheck;
        resetSMtimers();
      break;
    }
    
    // max 2 steps
    if(WVprocessStep == 3) { WVwateringCyles++; WVprocessStep = 0; } else { WVprocessStep++; }
    WVtimerNext = WVtimerLast + WVtimerRateCurrent;
    
  }
  
}

// ---------------------------
// serial output functions
// ---------------------------
// debug reporting
void debugReport() {
  
   Serial.println("");
   Serial.print("timerCurrentTime = "); Serial.print(timerCurrentTime);
   Serial.println("");
   Serial.print("SMsensorTimerRateCurrent = "); Serial.print(SMsensorTimerRateCurrent);
   Serial.println("");
   Serial.print("SMtimerLast = "); Serial.print(SMtimerLast);
   Serial.println("");
   Serial.print("SMtimerNext = "); Serial.print(SMtimerNext);
   Serial.println("");
   Serial.println("");
   
}

// this function called on message complete
void reportLevels(){
  
  Serial.print("== SM1 Moisture: ");
  Serial.print(SM1moisturePercent);
  Serial.print("% ==");
  Serial.println("");
  
  Serial.print("== Total soil moisture checks: ");
  Serial.print(SMsensorCycles);
  Serial.print(" ==");
  Serial.println("");
  
  Serial.print("== Total watering cycles: ");
  Serial.print(WVwateringCyles);
  Serial.print(" ==");
  Serial.println("");
  
  numbFlashes = (SM1moisturePercent/10); // flashes once for every 10 percent of moisture
  if(numbFlashes > 10) { numbFlashes = 10; } // max 10
  flashNumb = 0;
  
  do {
    digitalWrite(ledPin,HIGH);
    delay(100);
    flashNumb++;
  } while(numbFlashes > flashNumb);
  digitalWrite(ledPin,LOW);
  
}
