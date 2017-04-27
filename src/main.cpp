//----------------------------------------------------------------------
// https://github.com/clearwater/SwitecX25
// 
// This is an example of using the SwitchX25 library.
// It zero's the motor, sets the position to mid-range
// and waits for serial input to indicate new motor positions.
// 
// Open the serial monitor and try entering values 
// between 0 and 944.
// 
// Note that the maximum speed of the motor will be determined
// by how frequently you call update().  If you put a big slow
// serial.println() call in the loop below, the motor will move
// very slowly!
//----------------------------------------------------------------------

#include <Arduino.h>
#include <SwitecX25.h>

// standard X25.168 range 315 degrees at 1/3 degree steps
#define STEPS (315*3)

#define INTERRUPT_PIN 2

#define GEAR_RATIO 24/43

#define STEPS_PER_DEGREE 3

#define DEGREE_PER_RPM 30.0/1000.0

// For motors connected to digital pins 4,5,6,7
SwitecX25 motor1(STEPS,5,6,7,8);

unsigned long lastIntTime = 0;
unsigned long intTime = 0;

void interrupt(){
  lastIntTime = intTime;
  intTime = micros();
}

void setup(void)
{
  // run the motor against the stops
  motor1.zero();
  // start moving towards the center of the range
  //motor1.setPosition(STEPS/2);
  
  Serial.begin(9600);
  Serial.println("STARTING");

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt, RISING); 
}

void loop(void)
{
  //static int nextPos = 0;
  // the motor only moves when you call update
  motor1.update();

  unsigned long now = millis();

  static unsigned long last_calc = now;

  if(now - last_calc > 100){
    long delta = intTime - lastIntTime; 
    Serial.print(delta);
    Serial.print(":");

    double deltasec = delta/1000000.0; 
    Serial.print(deltasec);
    Serial.print(":");

    double pps = 1.0/deltasec;
    Serial.print(pps);
    Serial.print(":");
    
    double ppm = pps*60.0;
    Serial.print(ppm);
    Serial.print(":");
    
    double rpm = ppm/2.0;
    Serial.print(rpm);
    Serial.print(":");

    double steps = rpm * DEGREE_PER_RPM * STEPS_PER_DEGREE * GEAR_RATIO;
    Serial.print(steps);
    Serial.print(":");

    Serial.println();

    motor1.setPosition(steps);

    last_calc = now;
  }

  static unsigned long last_update = now;
  static bool last_pos = false;

  if(now - last_update > 1000){
    if(last_pos){
      //motor1.setPosition(0);
    }else{
      //motor1.setPosition(STEPS-1);
    }
    last_pos = !last_pos;
    last_update = now;
  }
  
  /*
  if (Serial.available()) {
    char c = Serial.read();
    if (c==10 || c==13) {
      motor1.setPosition(nextPos);
      nextPos = 0;
    } else if (c>='0' && c<='9') {
      nextPos = 10*nextPos + (c-'0');
    }
  }
  */
}

