//
// CX Digital Tachometer
// Tim Hollabaugh
//

#include <Arduino.h>
#include <SwitecX25.h>

#define DEBUG

#define LED 13
#define INTERRUPT_PIN 2

// standard X25.168 range 315 degrees at 1/3 degree steps
#define STEPS (315*3)

#define TIMEOUT 1000000UL

#define GEAR_RATIO 24/43
#define STEPS_PER_DEGREE 3
#define DEGREE_PER_RPM 32.5/1000.0

// The number of pulses to average
#define DELTAS 256

// Maximum allowed positive changes in delta per microsecond
#define MAX_DELTA_ACCELERATION 0.04

// Maximum allowed negitve changes in delta per microsecond
#define MAX_DELTA_DECCELERATION -1

// For motors connected to digital pins 4,5,6,7
SwitecX25 motor1(STEPS,5,6,7,8);

// Buffer for the rolling average of input pulses
//unsigned long deltas[DELTAS][2];
//int index = 0;

unsigned long lastIntTime = 0UL;
unsigned long intTime = 0UL;

unsigned long lastValidDelta = 0UL;
unsigned long lastValidTime = 0UL;

void interrupt(){
  
  // Falling edge
  intTime = micros();

  unsigned long delta = intTime - lastIntTime;

  unsigned long change_in_time = intTime - lastValidTime;
  long change_in_delta = delta - lastValidDelta;

  long max_change_in_delta = (float)MAX_DELTA_ACCELERATION*(float)change_in_time;

  if(change_in_delta > max_change_in_delta){
    delta = lastValidDelta + max_change_in_delta;
  }

  if(change_in_delta < -max_change_in_delta){
    delta = lastValidDelta - max_change_in_delta;
  }

  lastValidDelta = delta;
  lastValidTime = intTime;

  lastIntTime = intTime;

}

void setup(void){
  // run the motor against the stops
  motor1.zero();
  
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("STARTING");
#endif // DEBUG

  pinMode(LED, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT);

#ifdef DEBUG

  digitalWrite(LED, HIGH);
  motor1.setPosition(STEPS/2);
  motor1.updateBlocking();

  digitalWrite(LED, LOW);
  motor1.setPosition(0);
  motor1.updateBlocking();

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
#endif // DEBUG

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt, FALLING); 
}

void loop(void){

  motor1.update();

  unsigned long now = millis();

  static unsigned long last_calc = now;

  // Do the rpm calculations and update the motor every 50ms
  if(now - last_calc > 50){
    //noInterrupts();

    // Check if there was a pulse within the timeout
    if(micros() - intTime < TIMEOUT){
    //if(true){
      /*
      long delta = intTime - lastIntTime; 

      // Create a rolling average of the deltas
      unsigned long sum = 0;
      unsigned long count = 0;

      // Move everything down one spot
      for(int i = 0; i < DELTAS - 1; i++){
        deltas[i] = deltas[i+1];
        if(deltas[i] > 0){
          sum += deltas[i];
          count++;
        }
      }

      // Add the most recent delta to the end
      if(delta != 0){
        deltas[DELTAS - 1] = delta;
        sum += delta;
        count++;
      }
      */

      //unsigned long average_delta = sum/count;
      //double deltasec = average_delta/1000000.0; 
      double deltasec = lastValidDelta/1000000.0; 
      double pps = 1.0/deltasec;
      double ppm = pps*60.0;
      double rpm = ppm/2.0;
      double steps = rpm * DEGREE_PER_RPM * STEPS_PER_DEGREE * GEAR_RATIO;

#ifdef DEBUG
      Serial.print(lastValidDelta);
      Serial.print(":");
      Serial.print(lastValidTime);
      Serial.print(":");
      Serial.print(rpm);
      Serial.print(":");
      Serial.print(steps);
      Serial.println();
#endif // DEBUG

      motor1.setPosition(steps);
      //interrupts();
    }else{
      //interrupts();
      motor1.setPosition(0);
#ifdef DEBUG
      Serial.println("Timeout!");
#endif // DEBUG
    }

    last_calc = now;
  }
}

