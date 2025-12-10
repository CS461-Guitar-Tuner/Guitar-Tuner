#include <Adafruit_TinyUSB.h>

// Motor driver pins (to driver)
#define AIN1 A1   // PWM-capable pin
#define AIN2 A2   // PWM-capable pin
#define SLP  A3   // driver enable (sleep)

// Control pins from master (CP Classic)
#define DIR_IN  A5   // HIGH = tighten, LOW = loosen
#define EN_IN   A6   // HIGH = motor on, LOW = motor off

const int MOTOR_SPEED = 200;   // 0–255, constant speed

void setup() {
  Serial.begin(115200);
  Serial.println("Bluefruit Motor Slave - DIR/EN control");

  // Control inputs from master
  pinMode(DIR_IN, INPUT);
  pinMode(EN_IN, INPUT);

  // Motor driver outputs
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(SLP, OUTPUT);

  digitalWrite(SLP, HIGH);  // enable motor driver

  // Ensure motor is stopped at start
  analogWrite(AIN1, 0);
  analogWrite(AIN2, 0);
}

void loop() {
  int en  = digitalRead(EN_IN);
  int dir = digitalRead(DIR_IN);

  if (en == LOW) {
    // STOP motor
    analogWrite(AIN1, 0);
    analogWrite(AIN2, 0);
  } else {
    // RUN motor at constant speed
    if (dir == HIGH) {
      // TIGHTEN direction
      analogWrite(AIN1, MOTOR_SPEED);
      analogWrite(AIN2, 0);
    } else {
      // LOOSEN direction
      analogWrite(AIN1, 0);
      analogWrite(AIN2, MOTOR_SPEED);
    }
  }

  // Optional: tiny delay to chill
  // delay(5);
}
