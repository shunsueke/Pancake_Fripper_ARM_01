/*
  ESP32 PS3 Controller - 4 Servo Arm Control
*/

#include <Ps3Controller.h>
#include <ESP32Servo.h>

// Servo Pins
#define BASE_SERVO_PIN 27
#define ARM1_SERVO_PIN 26
#define ARM2_SERVO_PIN 33
#define HAND_SERVO_PIN 32

// Servo angle limits
#define BASE_MIN 0
#define BASE_MAX 270

#define ARM1_MIN 70
#define ARM1_MAX 170

#define ARM2_MIN 125
#define ARM2_MAX 190

#define HAND_MIN 60
#define HAND_MAX 150

// Servo objects
Servo base_servo;
Servo arm1_servo;
Servo arm2_servo;
Servo hand_servo;

// Current positions
int basePos = 90;  // 中央
int arm1Pos = 70;
int arm2Pos = 190;
int handPos = HAND_MIN;

// Button state
bool handToggled = false;
bool leftButtonPressed = false;

// Deadzone to prevent jitter
const int deadzone = 15;

// Callback when controller input changes
void notify() {
  // BASE SERVO - leftX joystick
  int lx = Ps3.data.analog.stick.lx;
  if (abs(lx) > deadzone) {
    basePos += map(lx, -128, 127, -3, 3);  // 調整速度
    basePos = constrain(basePos, BASE_MIN, BASE_MAX);
    base_servo.write(basePos);
  }

  // ARM1 SERVO - rightX joystick
  int rx = Ps3.data.analog.stick.rx;
  if (abs(rx) > deadzone) {
    arm1Pos += map(rx, -128, 127, -3, 3);
    arm1Pos = constrain(arm1Pos, ARM1_MIN, ARM1_MAX);
    arm1_servo.write(arm1Pos);
  }

  // ARM2 SERVO - rightY joystick
  int ry = Ps3.data.analog.stick.ry;
  if (abs(ry) > deadzone) {
    arm2Pos += map(ry, -128, 127, 3, -3);  // Yは上下逆
    arm2Pos = constrain(arm2Pos, ARM2_MIN, ARM2_MAX);
    arm2_servo.write(arm2Pos);
  }

  // HAND SERVO - L2 ボタンでトグル
  if (Ps3.event.button_down.l2 && !leftButtonPressed) {
    handPos = handToggled ? HAND_MIN : HAND_MAX;
    hand_servo.write(handPos);
    handToggled = !handToggled;
    leftButtonPressed = true;
  }
  if (Ps3.event.button_up.l2) {
    leftButtonPressed = false;
  }

  // デバッグ表示
  Serial.print("Base: "); Serial.print(basePos);
  Serial.print("  Arm1: "); Serial.print(arm1Pos);
  Serial.print("  Arm2: "); Serial.print(arm2Pos);
  Serial.print("  Hand: "); Serial.println(handPos);
}

// OnConnect callback
void onConnect() {
  Serial.println("PS3 Controller Connected!");
}

void setup() {
  Serial.begin(115200);

  // PS3 接続準備
  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("B0:B2:1C:A7:BD:22");  // ESP32のMACアドレス

  // Attach servos
  base_servo.attach(BASE_SERVO_PIN);
  arm1_servo.attach(ARM1_SERVO_PIN);
  arm2_servo.attach(ARM2_SERVO_PIN);
  hand_servo.attach(HAND_SERVO_PIN);

  // 初期位置
  base_servo.write(basePos);
  arm1_servo.write(arm1Pos);
  arm2_servo.write(arm2Pos);
  hand_servo.write(handPos);

  Serial.println("Ready.");
}

void loop() {
  if (!Ps3.isConnected()) return;
  delay(20);
}
