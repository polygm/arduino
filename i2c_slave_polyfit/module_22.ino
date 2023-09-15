// Polyfit version 2.2
#include <Wire.h>
#include "PinChangeInterrupt.h" // 라이브러리 추가 설치 필요



#define BEEP_PIN 12 // 부저

#define INTERRUPT0 2 // 인터럽트0
#define INTERRUPT1 3 // 인터럽트1

#define BUTTON1 8 // 외부버튼
#define BUTTON2 9 // 외부버튼

// 모터 연결핀
#define MOTOR_PIN1 7 // IN1
#define MOTOR_PIN2 6 // IN2
#define MOTOR_PIN3 5 // IN3
#define MOTOR_PIN4 4 // IN4

// 모터 48byj 스템모터
#define STEP_48BYJ 2048 // 1바퀴 스텝수

// 모터 회전 Loop 문 구현을 간단하게 하기위해 핀 배열 선언
int MotorPinArray[4] = { MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4 };

// CW 시계방향 데이터를 전송
int MOTOR_CW[4] = { 0b1000, 0b0100, 0b0010,0b0001 };
// CCW 반시계방향 데이터를 전송
int MOTOR_CCW[4] = { 0b0001, 0b0010, 0b0100, 0b1000 };

int motor_enable = 0;
int motor_angle_max = 0;
int motor_stop_max = 0;
int motor_position = 0;

int error = 0;
int itr0_status, itr1_status; //인터럽트 상태값

void beep(int time=1000)
{
  digitalWrite(BEEP_PIN, HIGH);
  delay(time);
  digitalWrite(BEEP_PIN, LOW);
  delay(time);
}

// 모터 48byj 스템모터 딜레이
void stepDelay()
{
  // 모터 부하가 없는 상태에서, 
  // 각 스텝 간격이 최소 3ms 이상이어야 정상적으로 동작합니다.
  delay(3);
}

int set_interrup0()
{
  pinMode(INTERRUPT0, INPUT);
  // 포토센서를 위한 인터럽트
  attachInterrupt( 
    digitalPinToInterrupt(INTERRUPT0), 
    itr0_rising, 
    RISING); // RISING : low -> high
  Serial.print("Interrupt 0 =");
  itr0_status = digitalRead(INTERRUPT0);
  Serial.println(itr0_status);

  return itr0_status;
}

int set_interrup1()
{
  pinMode(INTERRUPT1, INPUT);
  // 포토센서를 위한 인터럽트
  attachInterrupt( 
    digitalPinToInterrupt(INTERRUPT1), 
    itr1_rising, 
    RISING); // RISING : low -> high
  Serial.print("Interrupt 1 =");
  itr1_status = digitalRead(INTERRUPT1);
  Serial.println(itr1_status);

  return itr1_status;
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(BEEP_PIN, LOW);
  pinMode(BEEP_PIN, OUTPUT);
  
  // 초기화 beep음
  beep(100); beep(100); 
  

  // 시리얼 초기화
  Serial.begin(115200);
  Serial.println("Module On");


  int itr0 = set_interrup0();
  int itr1 = set_interrup1();
  if( itr0 == 0 && itr1 == 0) {
    error = 100;
  }

  set_motor();

  pinMode(BUTTON1,INPUT);
  attachPCINT( digitalPinToPCINT(BUTTON1), button1_press, FALLING);
  pinMode(BUTTON2,INPUT);
  attachPCINT( digitalPinToPCINT(BUTTON2), button2_press, FALLING);


  // i2c 설정
  Wire.begin(1);
  Wire.onReceive(receivedEvent);


  Serial.println("Module Ready");
  Serial.println("m=move, d=max_distance, c=current_position");
  Serial.println("------------------------------------------");

}

int button1_status = 0;
int button2_status = 0;
void button1_press() {
  button1_status = 1;
  Serial.print("b1=");
  Serial.println(digitalRead(BUTTON1));
}
void button2_press() {
  button2_status = 1;
  Serial.print("b2=");
  Serial.println(digitalRead(BUTTON2));
}

void set_motor()
{
  Serial.println("Motor Checking...");

  // 모터 초기화
  // 스텝모터 드라이브 보드의 IN 연결핀 출력으로 설정
  for(int i = 0 ; i < 4 ; i++ ) {
    pinMode(MotorPinArray[i], OUTPUT);
  }

  // 최소치값으로 이동(시작점)
  motor_check_min();
  beep(100);

  // 최대치 이동 (스탭/각도 측정)
  motor_check_max();
  beep(100); 

  // 다시 시작 위치로 이동
  itr0_status = digitalRead(INTERRUPT0);
  if(itr0_status != 0) {
    motor_move(0); // 초기점으로 이동
    beep(100); beep(100);
  } else {
    // 시작 센서 오픈, 초기 시점으로 되돌리수 없음
    error = 101;
  }
  
}

int motor_move(int pos) {
  int distance;
  int temp;

  if(pos >= 0) {

    distance = pos - motor_position; // 이동할 위치 계산
    if(pos > motor_stop_max) {
      Serial.println("Maximum position exceeded");
      beep(2000);beep(300);beep(300);
      return 0;
    }

    Serial.print("Move to = ");
    Serial.print(pos);
    Serial.print(", current = ");
    Serial.print(motor_position);


    if(distance > 0 ) {
      // 정방향
      Serial.print(", distance = ");
      Serial.println(distance);
      for(int i = 0 ; i < distance ; i++){
        motor_position++; // 현재 모터 위치
        temp = i % 4;
        digitalWrite(MotorPinArray[0], bitRead(MOTOR_CCW[temp], 0));
        digitalWrite(MotorPinArray[1], bitRead(MOTOR_CCW[temp], 1));
        digitalWrite(MotorPinArray[2], bitRead(MOTOR_CCW[temp], 2));
        digitalWrite(MotorPinArray[3], bitRead(MOTOR_CCW[temp], 3));
        stepDelay();

        //Serial.println(motor_position);
      }
    } else {
      // 역방향
      distance = distance * (-1);
      Serial.print(", distance = ");
      Serial.println(distance);
      for(int i = 0 ; i < distance ; i++){
        motor_position--; // 현재 모터 위치
        temp = i % 4;
        digitalWrite(MotorPinArray[0], bitRead(MOTOR_CW[temp], 0));
        digitalWrite(MotorPinArray[1], bitRead(MOTOR_CW[temp], 1));
        digitalWrite(MotorPinArray[2], bitRead(MOTOR_CW[temp], 2));
        digitalWrite(MotorPinArray[3], bitRead(MOTOR_CW[temp], 3));
        stepDelay();

        //Serial.println(motor_position);
      }
    }

  } else {
    Serial.println("The location cannot be determined.");
    //beep(2000);beep(300);beep(300);
  }

  return motor_position;
}

void motor_check_max() {
  int temp;

  motor_enable = 1;
  while(motor_enable) {
    for(int i = 0 ; i < 2048 ; i++){
      itr1_status = digitalRead(INTERRUPT1);
      if(itr1_status == 0) { // 우측 센서검출
        Serial.print("motor max position = ");
        Serial.println(motor_stop_max);
        motor_enable = 0;
        break;
      }

      motor_stop_max++; // 모터 거리측정
      motor_position++; // 현재 모터 위치

      temp = i % 4;
      digitalWrite(MotorPinArray[0], bitRead(MOTOR_CCW[temp], 0));
      digitalWrite(MotorPinArray[1], bitRead(MOTOR_CCW[temp], 1));
      digitalWrite(MotorPinArray[2], bitRead(MOTOR_CCW[temp], 2));
      digitalWrite(MotorPinArray[3], bitRead(MOTOR_CCW[temp], 3));
      stepDelay();

      //Serial.println(i);
    }
  }
}

// 모터 최소 시작위치
void motor_check_min() {
  int temp;
  // 최대위치로 모터 이동
  motor_enable = 1;
  while(motor_enable) {
    for(int i = 0 ; i < 2048 ; i++){
      itr0_status = digitalRead(INTERRUPT0);
      if(itr0_status == 0) { // 좌측 센서검출
        Serial.println("motor start position");
        motor_enable = 0;
        motor_position = 0;
        break;
      }

      temp = i % 4;
      digitalWrite(MotorPinArray[0], bitRead(MOTOR_CW[temp], 0));
      digitalWrite(MotorPinArray[1], bitRead(MOTOR_CW[temp], 1));
      digitalWrite(MotorPinArray[2], bitRead(MOTOR_CW[temp], 2));
      digitalWrite(MotorPinArray[3], bitRead(MOTOR_CW[temp], 3));
      stepDelay();

      //Serial.println(i);
    }
  }
}




int itr0_cnt = 0;
int itr1_cnt = 0;
void itr0_rising()
{
  int itr0;
  itr0 = digitalRead(INTERRUPT0);

  // 인터럽트 변화 검사
  if(itr0_status != itr0) {
    //Serial.print(itr0_cnt++);
    //Serial.print("Interrupt 0 =");
    //Serial.println(itr0);

    // ===== 동작로직 =====
    // High -> Low로 이동시
    if(itr0_status == 1 && itr0 == 0) {
      Serial.println("Senser0 checking. Motor Min stop");
    }

    if(itr1_status == 0 && itr0 == 0) {
      error = 100; // 상판 오픈
      Serial.print("itr1_status="); Serial.print(itr1_status);
      Serial.println(" cover open");
    }

    // 변화된 itr0 값을 재기록
    itr0_status = itr0; 
  }
}

void itr1_rising()
{
  int itr1;
  itr1 = digitalRead(INTERRUPT1);

  // 인터럽트 변화 검사
  if(itr1_status != itr1) {
    //Serial.print(itr1_cnt++);
    //Serial.print("Interrupt 1 =");
    //Serial.println(itr1);

    // ===== 동작로직 =====
    // High -> Low로 이동시
    if(itr1_status == 1 && itr1 == 0) {
      Serial.println("Senser1 checking. Motor Max stop");
    }

    if(itr0_status == 0 && itr1 == 0) {
      error = 100; // 상판 오픈
      Serial.println("cover open");
    }


    // 변화된 itr0 값을 재기록
    itr1_status = itr1; 
  }
}

/**
 * 내부 LED 깜빡임 (초기값 1초)
*/
void builtin_led_blank(int time=1000) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(time);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(time);                      // wait for a second
}

/**
 * I2C를 통하여 데이터 값을 전달 받음
*/
int received_pos = 0;
void receivedEvent(int howmany) {
  int x;
  x = Wire.read();
  Serial.print(x);
  
  // 숫자값 계산 확인
  // 48->0, 57->9
  if( x >= 48 && x <= 57) {
    received_pos = received_pos * 10 + (x - 48);
    Serial.print("position = ");
    Serial.println(received_pos);
  }

  // m(109) 값 전달 받은 경우 이동
  if(x == 109) {
    //move = 1;
    motor_move(received_pos);
    received_pos = 0;
  }

}

void loop() {
  int btn1, btn2;
  // put your main code here, to run repeatedly:
  builtin_led_blank(500); // 0.5초 깜빡임

  // ===== 오류체크
  // 포터센서 모두 오픈, 상판 미연결
  if(error != 0) {
    beep(300);
    Serial.print(error);
    Serial.println(":poto sensor open!!!");

    itr0_status = digitalRead(INTERRUPT0);
    itr1_status = digitalRead(INTERRUPT1);

    // 상판 연결시 오류 해제
    if(itr0_status == 1 || itr1_status == 1) {
      error = 0;
    }
  }


  while(button1_status) {
    btn1 = digitalRead(BUTTON1);
    if(btn1 == 0) {
      Serial.print(".");
      motor_move(motor_position+5);
      beep(150);
    } else {
      Serial.println("up");
      button1_status = 0;
    }
  }

  while(button2_status) {
    btn2 = digitalRead(BUTTON2);
    if(btn2 == 0) {
      Serial.print(".");
      motor_move(motor_position-5);
      beep(200);
    } else {
      Serial.println("up");
      button2_status = 0;
    }
  }

  


  char cmd;
  if(Serial.available()){
    cmd = Serial.read();

    if(cmd == 10) { 
      // lf code (10)
    } else if( cmd >= 48 && cmd <= 57) {
      received_pos = received_pos * 10 + (cmd - 48);
      //Serial.print("position = ");
      //Serial.println(received_pos);
      Serial.print(cmd);
    } else if(cmd == 109) { // m(109) 값 전달 받은 경우 이동
      motor_move(received_pos);
      received_pos = 0;
    } else if(cmd == 100) { // d
      Serial.print("max distance = ");
      Serial.println(motor_stop_max);
    } else if(cmd == 99) { // c
      Serial.print("current position = ");
      Serial.println(motor_position);
    }

/*
    // isAlphaNumeric(thisChar)
    // isDigit(cmd)

    cmd = Serial.parseInt();
    Serial.print(motor_position);
    Serial.print(" to move ");
    Serial.print(cmd);
    Serial.print(" of ");
    Serial.print(motor_stop_max);

    motor_move(cmd);

    // lf (10)
    cmd = Serial.read(); //LF 코드 읽기
    */
  }
  
}

