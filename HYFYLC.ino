#include "MyTimer.h"

// Pin defines
#define btn1Pin 2 // button 1
#define btn2Pin 3 // button 2
#define debounceTime 200 // for buttons
#define comboTime 2000 // for buttons

#define mA1A 6  // Motor A signal 1A (spring charger)
#define mA1B 7  // Motor A signal 1B (spring charger)
#define mB1A 8  // Motor B signal 1A (food load)
#define mB1B 9  // Motor B signal 1B (food load)

#define lightBarrierPin A0  // analog input voltage divider
#define ledPin 12 // status LED


// Time and thresholds defines
#define loadTime 4000
#define unclogTime 1500
#define loadAttempts 5
#define springChargeTime 1000
#define lightBaseline 100
#define lightThreshold 80
#define baselineTolerance 20


/*
 * state:
 * 0 - ilde: wait for start - led on
 * 1 - load food: activate motor B CW and check for food transit
 * 2 - unclog: activate motor B CCW
 * 3 - launch: activate motor A CW for a certain amount of time
 * 4 -
 * 5 -
 * 
 */

MyTimer btn1Tmr;
MyTimer btn2Tmr;
MyTimer btnComboTmr;
MyTimer ledTmr;
MyTimer mtrATmr;
MyTimer mtrBTmr;

int state = 0;
bool first = true;

bool btn1, prevBtn1, btn2, prevBtn2, combo;

bool motorAStatus = 0;
bool motorBStatus = 0;
byte loadCount = 0;
byte unclogCount = 0;
int lightReference = lightBaseline;
int delta = 0, prevDelta = 0;


int ledTime[] = {3000, 3000};
int ledIndex = 0;

void LedSet(int A = 0, int B = 0);
void MotorCmd(bool n, bool A = 0, bool B = 0, int T = 0 );
int LightTest();

void setup() {
  Serial.begin(9600); // 
  // Pin initialization:
  pinMode(btn1Pin, INPUT);  
  pinMode(btn2Pin, INPUT);
  
  pinMode(mA1A, OUTPUT);
  pinMode(mA1B, OUTPUT);
  pinMode(mB1A, OUTPUT);
  pinMode(mB1B, OUTPUT);

  pinMode(ledPin, OUTPUT);

  btn1Tmr.set(debounceTime);
  btn1Tmr.start();
  btn1 = digitalRead(btn1Pin);
  prevBtn1 = btn1;
  btn2Tmr.set(debounceTime);
  btn2Tmr.start();
  btn2 = digitalRead(btn2Pin);
  prevBtn2 = btn2;
  combo = 0;
      
  digitalWrite(mA1A, LOW);
  digitalWrite(mA1B, LOW);
  digitalWrite(mB1A, LOW);
  digitalWrite(mB1B, LOW);

  lightReference = LightTest();
  Serial.print("Light references: ");
  Serial.println(lightReference);
  LedSet(900, 100);
  
}

void loop() {
  prevBtn1 = btn1;
  prevBtn2 = btn2;
  

  if (!ledTmr.check()){LedSet(ledTime[0],   ledTime[1]);}
  if (motorAStatus && !mtrATmr.check()){MotorCmd(0, 0, 0, 0);}
  if (motorBStatus && !mtrBTmr.check()){MotorCmd(1, 0, 0, 0);}
  
  switch(state){
    case 0:
      /* Idle state, wait for button 1 press:
       * - button 1 -> start the device
       * - button 1 + button 2 pressed for 2 sec (see comboTime define) -> calibration mode
      */
      if (first){
        // run code only once
        first = false;
        Serial.println(state);
        LedSet(900, 100);
        loadCount = 0;
        unclogCount = 0;

      }
      // 
      //Serial.println("...");
      btn1 = digitalRead(btn1Pin);
      btn2 = digitalRead(btn2Pin);
      if ((!btn1Tmr.check()) && (prevBtn1 && !btn1) && (!combo)){
        btn1Tmr.set(debounceTime);
        btn1Tmr.start();
        // Serial.println("...");
        go(1);
      }
      if ((!btnComboTmr.check()) && (btn2 && btn1) && (!combo)){
        btnComboTmr.set(comboTime);
        btnComboTmr.start();
        combo = true;
        
      }
      if ((!btnComboTmr.check()) && (combo)){
        combo = false;
        // Serial.println("...");
        go(5);
      }
      break;
    case 1:
      /* Loading state: load food and check for transit and count loading attempt
       * - button 1 -> go to idle (state 0)
       * - button 2 -> go to unclog (state 2)
      */
      if (first){
        // run code only once
        first = false;
        Serial.println(state);
        LedSet(1, 1);
      }
      //
      //Serial.println("...");
      if (loadCount == loadAttempts){
        loadCount = 0;
        unclogCount = 0;
        MotorCmd(1, 0, 0, loadTime);
        go(0);
      }
      btn1 = digitalRead(btn1Pin);
      btn2 = digitalRead(btn2Pin);
      if ((!btn1Tmr.check()) && (prevBtn1 && !btn1) ){
        btn1Tmr.set(debounceTime);
        btn1Tmr.start();
        // Serial.println("...");
        MotorCmd(1, 0, 0, loadTime);
        go(0);
      }
      if ((!btn2Tmr.check()) && (prevBtn2 && !btn2) ){
        btn2Tmr.set(debounceTime);
        btn2Tmr.start();
        MotorCmd(1, 0, 0); // Stops and wait to avoid arduino reset
        delay(500);
        MotorCmd(1, 1, 0, unclogTime);
        // Serial.println("...");
        //go(2);
      }
      if (!motorBStatus && !mtrBTmr.check()) {
        if (loadCount == unclogCount){
          MotorCmd(1, 0, 1, loadTime); // CW
          loadCount += 1;
          Serial.print("Load count: ");
          Serial.println(loadCount);
        } else {
          go(2);
          unclogCount += 1;
        }
      }
      if (100-millis()%100 < 200){
         Serial.println(analogRead(lightBarrierPin));
      }
      if ((lightReference-analogRead(lightBarrierPin)) > lightThreshold) {
        MotorCmd(1, 0, 0); // Stops and wait to avoid arduino reset
        delay(500);
        go(3);
      }
      break;
    case 2:
      if (first){
        // run code only once
        first = false;
        Serial.println(state);
        LedSet(200, 200);
        MotorCmd(1, 0, 0);
        delay(500);
        MotorCmd(1, 1, 0, unclogTime);
      }
      btn1 = digitalRead(btn1Pin);
      btn2 = digitalRead(btn2Pin);
      if ((!btn1Tmr.check()) && (prevBtn1 && !btn1) ){
        btn1Tmr.set(debounceTime);
        btn1Tmr.start();
        // Serial.println("...");
        MotorCmd(1, 0, 0, loadTime);
        go(0);
      }
      if (!motorBStatus && !mtrBTmr.check()) {
        MotorCmd(1, 0, 0); // Stops and wait to avoid arduino reset
        delay(500);
        go(1);
      }
      //go(3);
      
      break;
    case 3:
      if (first){
        // run code only once
        Serial.println(state);
        MotorCmd(0, 0, 1, springChargeTime);
      }
      btn1 = digitalRead(btn1Pin);
      //btn2 = digitalRead(btn2Pin);
      if ((!btn1Tmr.check()) && (prevBtn1 && !btn1) ){
        btn1Tmr.set(debounceTime);
        btn1Tmr.start();
        // Serial.println("...");
        go(0);
      }
      if (!motorAStatus && !mtrATmr.check()) {
        loadCount = 0;
        unclogCount = 0;
        go(1);
      }
      //
      //Serial.println("...");
      //go(4);
      break;
    case 4:
      if (first){
        // run code only once
        first = false;
        //Serial.println(state);
      }
      //
      //Serial.println("...");
      //go(5);
      break;
    case 5:
      if (first){
        // run code only once
        first = false;
        Serial.println(state);
        LedSet(1, 1);
        while(digitalRead(btn2Pin)){}
        LedSet(500, 500);
        btn2Tmr.set(debounceTime);
        btn2Tmr.start();
        LedSet(250 , 250);
      }
      btn2 = digitalRead(btn2Pin);
      if ((!btn2Tmr.check()) && (prevBtn2 && !btn2)){
        btn2Tmr.set(debounceTime);
        btn2Tmr.start();
        // Serial.println("...");
        btn1Tmr.set(debounceTime); //???
        btn1Tmr.start(); //???
        lightReference = analogRead(lightBarrierPin);
        Serial.print("lightReference: ");
        Serial.println(lightReference);
        go(0);
      }      //
      delta =  abs(analogRead(lightBarrierPin)-lightBaseline);

      if (abs(delta-prevDelta) > 5){
        if (delta <=  baselineTolerance){
          LedSet(1 , 1);
        } else {   
          LedSet(250 , 250);
        }
        prevDelta = delta;
        Serial.print("Delta: ");
        Serial.println(delta);
      }
 
      
      //Serial.println("...");

      break;
  }
}

void go(int s){
   state = s;
   first = true;
   delay(100);
}


void LedSet(int A = 0, int B = 0){
  if ((A>0) && (B>0)){
    ledTime[0] = A; ledTime[1] = B;
    ledIndex = !ledIndex;
    ledTmr.set(ledTime[ledIndex]);
    ledTmr.start();
    digitalWrite(ledPin, ledIndex);
  } else if ((A == 1) && (B==1) ){
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);

  }
}

void MotorCmd(bool n, bool A, bool B, int T){
  switch (n){
  case 0 :
    motorAStatus = 0;
    digitalWrite(mA1A, A);
    digitalWrite(mA1B, B);
    if (A != B ){
      mtrATmr.set(T);
      mtrATmr.start();
      motorAStatus = 1;
    }
    break;
  case 1 :
    motorBStatus = 0;
    digitalWrite(mB1A, A);
    digitalWrite(mB1B, B);
    if (A != B ){
      mtrBTmr.set(T);
      mtrBTmr.start();
      motorBStatus = 1;
    }
    break;
 
  }
}

int LightTest(){
  LedSet(1, 1);
  long lValue = 0;
  
  for (int i=0; i<100; i++){
    lValue = lValue + analogRead(lightBarrierPin);
    delay(20);
  }
  return (int)(lValue/100);
  
}
