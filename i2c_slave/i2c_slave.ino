/*
 * I2c Slave 모듈 코드
 * version 0.0.1
*/
#include <Wire.h>
#define BEEP_PIN 12 // 부저

/**
 * 내부 LED 깜빡임 (초기값 1초)
*/
void builtin_led_blank(int time=1000) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(time);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(time);                      // wait for a second
}

void beep(int time)
{
  // 초기화2 : 비프 소리
  digitalWrite(BEEP_PIN, HIGH);
  delay(time);
  digitalWrite(BEEP_PIN, LOW);
  delay(time);
}



void setup() {
  // 초기화1 : 내부 LED, Beep
  pinMode(LED_BUILTIN, OUTPUT);

  // 초기화2. 전원 입력시 삑 (1번)
  pinMode(BEEP_PIN, OUTPUT);
  beep(200); // 삐빅

  Wire.begin(1);                // join I2C bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  Serial.begin(2000000);          // start serial for output

  Serial.println("Slave Ready");
}

/**
 * I2C를 통하여 데이터 값을 전달 받음
*/
void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  
  char c = Wire.read(); // receive byte as a character
  Serial.println(c); 

  //int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);         // print the integer
}

void requestEvent() {
  Wire.write("hello "); // respond with message of 6 bytes
  // as expected by master
}

void loop() {


}
