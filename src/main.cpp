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

#define TIMEOUT 1000

#define GEAR_RATIO 24/43
#define STEPS_PER_DEGREE 3
#define DEGREE_PER_RPM 30.0/1000.0

// The number of pulses to average
#define DELTAS 20

// For motors connected to digital pins 4,5,6,7
SwitecX25 motor1(STEPS,5,6,7,8);

// Buffer for the rolling average of input pulses
unsigned long deltas[DELTAS];

unsigned long lastIntTime = 0;
unsigned long intTime = 0;

void interrupt(){
  
  // Only get the pulse length on the falling edge
  // The falling edge is steeper and more accurate
  
  if(digitalRead(INTERRUPT_PIN)){
    digitalWrite(LED, HIGH);
  }else{
    lastIntTime = intTime;
    intTime = micros();
    digitalWrite(LED, LOW);
  }
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

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt, CHANGE); 
}

void loop(void){

  motor1.update();

  unsigned long now = millis();

  static unsigned long last_calc = now;

  // Do the rpm calculations and update the motor every 50ms
  if(now - last_calc > 50){

    // Check if there was a pulse within the timeout
    if(now - intTime < TIMEOUT){
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

      unsigned long average_delta = sum/count;
      double deltasec = average_delta/1000000.0; 
      double pps = 1.0/deltasec;
      double ppm = pps*60.0;
      double rpm = ppm/2.0;
      double steps = rpm * DEGREE_PER_RPM * STEPS_PER_DEGREE * GEAR_RATIO;

#ifdef DEBUG
      Serial.print(delta);
      Serial.print(":");
      Serial.print(average_delta);
      Serial.print(":");
      Serial.print(rpm);
      Serial.print(":");
      Serial.print(steps);
      Serial.println();
#endif // DEBUG

      motor1.setPosition(steps);
    }else{
      motor1.setPosition(0);
    }

    last_calc = now;
  }
}

