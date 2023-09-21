/*
 * DC 모듈 코드
 * version 0.0.1
*/
#include <Wire.h>
#include <EEPROM.h>
#include <Servo.h>
#include "PinChangeInterrupt.h" // 라이브러리 추가 설치 필요

#define BeepPin 12 // 부저

// L298n DC-모터연결
#define DC1_1 4 // IN1
#define DC1_2 5 // IN2, PWM
#define DC2_1 6 // IN3, PWM
#define DC2_2 7 // IN4

#define BTN1 8 
#define BTN2 9 
#define BTN3 10 
#define BTN4 11 

#define PHOTO_ITR_TYPE 1

#define MOTOR1_ENCODER_A 2
#define MOTOR1_ENCODER_B A0
#define MOTOR2_ENCODER_A 3
#define MOTOR2_ENCODER_B A1

#define MOTOR1_ITR_A A2
#define MOTOR1_ITR_B A3

// 서보모터(PWM)
Servo servo_motor1, servo_motor2, servo_motor3;
Servo servo_motor4, servo_motor5, servo_motor6;
int server1_angle=0;
int server2_angle=0;
int server3_angle=0;
int server4_angle=0;
int server5_angle=0;
int server6_angle=0;

int motor1_init = 0;
int motor1_inited = 0; // 초기화 작업이 진행되었는지 여부 확인
int motor1_position = 0;
int motor1_direction = 1; // 1:정방향, 0:역방향
int motor1_max = 0;
String dc1_direction_type;
int motor1_cw_bit;
int motor1_ccw_bit;

int motor2_init = 0;
int motor2_inited = 0; // 초기화 작업이 진행되었는지 여부 확인
int motor2_position = 0;
int motor2_direction = 1; // 1:정방향, 0:역방향
int motor2_max = 0;
String dc2_direction_type;
int motor2_cw_bit;
int motor2_ccw_bit;

// 모터속도 지정
int m1_speed = 255, m2_speed=255;  // 모터 스피드
int m1_target = 0, m2_target = 0; // 이동해야 되는 위치
int m1_manual = 0, m2_manual = 0;

int time_blank_delay[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int timedelay = 0;
int time_led_toggle = 0;
int time_beep_toggle = 0;

int status = 0;
int slave_num;

String wire_input;

void setup() {
  // 전원 입력시 삑 (1번)
  pinMode(BeepPin, OUTPUT);
  beep(150); delay(150); // 삐빅


  //초기화3 : 시리얼
  // 시리얼 초기화
  Serial.begin(115200);
  Serial.println("Module On");

  // 초기화4. DC모터 초기화
  pinMode(DC1_1, OUTPUT);
  pinMode(DC1_2, OUTPUT);
  pinMode(DC2_1, OUTPUT);
  pinMode(DC2_2, OUTPUT);


  // 초기화5. 버튼스위치
  /*
  pinMode(BTN1, INPUT);
  attachPCINT( digitalPinToPCINT(BTN1), button1_press, FALLING);

  pinMode(BTN2, INPUT);
  attachPCINT( digitalPinToPCINT(BTN2), button2_press, FALLING);

  pinMode(BTN3, INPUT);
  attachPCINT( digitalPinToPCINT(BTN3), button3_press, FALLING);

  pinMode(BTN4, INPUT);
  attachPCINT( digitalPinToPCINT(BTN4), button4_press, FALLING);

  */

  // 초기화6, DC모터 엔코더
  pinMode(MOTOR1_ENCODER_A, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTOR1_ENCODER_A), ISR_motor1, FALLING);
  pinMode(MOTOR2_ENCODER_A, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTOR2_ENCODER_A), ISR_motor2, FALLING);


  pinMode(MOTOR1_ITR_A, INPUT);
  attachPCINT( digitalPinToPCINT(MOTOR1_ITR_A), motor1_potho_a, FALLING);
  pinMode(MOTOR1_ITR_B, INPUT);
  attachPCINT( digitalPinToPCINT(MOTOR1_ITR_B), motor1_potho_b, FALLING);
  


  // 초기화8: i2c 통신 설정, (A4, A5)
  // 127개 통신
  Wire.begin(20); // 각각 모듈 번호 지정
  Wire.onReceive(receiveEvent);
 

  

  // 서보모터 초기화
  Serial.println("servo4 init");
  servo_motor4.attach(9);
  servo_motor4.write(0);
  delay(20);

  Serial.println("servo5 init");
  servo_motor5.attach(10);
  servo_motor5.write(0);
  delay(20);

  // 초기화7 : 모터센서 초기화
  //motor_dc1_init();


  /*
  // 부저 예약
  Serial.println("servo6 init");
  servo_motor6.attach(11);
  servo_motor6.write(0);
  delay(20);
  */
 
  
}

/**
 * I2C를 통하여 데이터 값을 전달 받음
*/
void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    wire_input += c;
    Serial.print(c);         // print the character
  }
  
  char c = Wire.read(); // receive byte as a character
  wire_input += c;
  Serial.println(c); 

  //int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);         // print the integer
}

void requestEvent() {
  Wire.write("hello "); // respond with message of 6 bytes
  // as expected by master
}


void beep(int time)
{
  // 초기화1 : 내부 LED
  pinMode(LED_BUILTIN, OUTPUT);

  // 초기화2 : 비프 소리
  digitalWrite(BeepPin, HIGH);
  delay(time);
  digitalWrite(BeepPin, LOW);
}

// 모터 초기화
void motor_dc1_init() {
  int temp;
  int sensor1, sensor2;

  motor1_init = 1;

  m1_speed = 80; // 최소 속도
  m1_manual = 1;

  Serial.println("** motor init **");
  sensor1 = digitalRead(MOTOR1_ITR_A);
  Serial.print("sensor1=");
  Serial.println(sensor1);

  if(sensor1 == PHOTO_ITR_TYPE) {
    Serial.println("motor center!!!");
    M1_init_center_move();
    
  } else {
    Serial.print("motor is not center>");
    M1_init_uncenter_move();
    M1_init_center_move();
  }

  Serial.print("motor1_max = ");
  Serial.println(motor1_max);

  Serial.print("motor direction=");
  Serial.println(motor1_direction);

  //motor1_position = 0;  // 초기화   
  m1_manual = 0;

  motor1_init = 0;
  m1_manual = 0;

  Serial.print("motor1_cw_bit=");
  Serial.print(motor1_cw_bit);
  Serial.print(", motor1_ccw_bit=");
  Serial.println(motor1_ccw_bit);

  Serial.print("motor1_position=");
  Serial.print(motor1_position);

  beep(300);
  
  motor1_max = motor1_position;

  Serial.print("motor1_max=");
  Serial.print(motor1_max);

  motor1_position=0;

  Serial.print("motor1_position=");
  Serial.print(motor1_position);

  m1_speed = 255; // 속도 최대값 재지정

  // 모터 초기화가 완료됨
  motor1_inited = 1;
}

void motor_dc2_init() {
//
  // 모터 초기화가 완료됨
  motor2_inited = 1;
}

void M1_init_uncenter_move() {
  int sensor1, sensor2;

  noInterrupts();
  M1_CCW();
  Serial.print(dc1_direction_type);
  Serial.print(">");

  // 반대쪽 센서까지 이동
  while(1) { // 참조건
    sensor1 = digitalRead(MOTOR1_ITR_A);
    if(sensor1 == PHOTO_ITR_TYPE) {
      Serial.print("M1_CCW>");
      M1_stop();      
      break;
    }
  }

  Serial.print(", motor1_position=");
  Serial.print(motor1_position);

  interrupts(); 
}


void M1_init_center_move() {
  int sensor1, sensor2;
    noInterrupts();

    // 센서 이탈: 초기 센서가 감지되어 있는 경우
    // 모터를 조금 이동
    M1_CW();
    Serial.print(dc1_direction_type);
    Serial.print(">");

    // 이탈되었는지 확인
    while(1) { // 참조건
      sensor1 = digitalRead(MOTOR1_ITR_A);
      if(sensor1 != PHOTO_ITR_TYPE) {
        break;
      }
    }

    // 반대쪽 센서까지 이동
    while(1) { // 참조건
      sensor1 = digitalRead(MOTOR1_ITR_A);
      if(sensor1 == PHOTO_ITR_TYPE) {
        Serial.print("M1_CCW>");
        M1_stop();      
        break;
      }
    }

    // 거리측정 시점
    delay(300);
    interrupts();
    motor1_position = 0;
    Serial.print("motor1_position init =");
    Serial.print(motor1_position);
    
    
    // 역이동
    M1_CCW();
    Serial.print(dc1_direction_type);
    Serial.print(">");

    // 이탈되었는지 확인
    while(1) { // 참조건
      sensor2 = digitalRead(MOTOR1_ITR_B);
      if(sensor2 != PHOTO_ITR_TYPE) {
        break;
      }
    }

    // 반대쪽 센서까지 이동
    while(1) { // 참조건
      sensor2 = digitalRead(MOTOR1_ITR_B);      
      if(sensor2 == PHOTO_ITR_TYPE) {
        Serial.print("M1_CCW>");
        M1_stop();      
        break;
      }
    }

    //motor1_position = 0;
    //motor1_max = motor1_position;

       
    
}


int m1_a_status = 0;
int m1_b_status = 0;

void motor1_potho_a() {
  int temp = digitalRead(MOTOR1_ITR_A);
  if(temp == 0) {
    m1_a_status = 1;
    //Serial.print(temp);
    //Serial.println(": motor1 photo a = open");
  }
}

void motor1_potho_b() {
  int temp = digitalRead(MOTOR1_ITR_B);
  if(temp == 0) {
    m1_b_status = 1;
    //Serial.print(temp);
    //Serial.println(": motor1 photo b = open");
  }
}



// 신호가 들어오면 동작하는 함수, 인터럽트
void ISR_motor1() {
  // 방향체크
  byte encoderB = digitalRead(MOTOR1_ENCODER_B);
  if(motor1_init == 1) {
    if(dc1_direction_type == "CW") {
      motor1_cw_bit = encoderB;
      motor1_position--; //카운트
    }

    if(dc1_direction_type == "CCW") {
      motor1_ccw_bit = encoderB;
      motor1_position++; //카운트
    }
    
    motor1_direction = 1; // 모터 회전 방향


  } else {
    if(dc1_direction_type == "CW") {
      motor1_cw_bit = encoderB;
      motor1_position--; //카운트
    }

    if(dc1_direction_type == "CCW") {
      motor1_ccw_bit = encoderB;
      motor1_position++; //카운트
    }
    
    motor1_direction = 1; // 모터 회전 방향

    if(m1_manual == 1) {

    } else {

      // 거리측정 이동/정지
      if(dc1_direction_type == "CCW") {
        if(motor1_position > m1_target) {
          M1_stop();
          Serial.print("ISR STOP CCW >> motor1_position = ");
          Serial.print(motor1_position);
          Serial.print(",target = ");
          Serial.println(m1_target);
        }
      }

      if(dc1_direction_type == "CW") {
        if(motor1_position < m1_target) {
          M1_stop();
          Serial.print("ISR STOP CW>> motor1_position = ");
          Serial.print(motor1_position);
          Serial.print(",target = ");
          Serial.println(m1_target);
        }
      }
        
    }
  }

}

void ISR_motor2() {
  // 방향체크
  byte encoderB = digitalRead(MOTOR2_ENCODER_B);
  if(motor2_init == 1) {
    if(dc2_direction_type == "CW") {
      motor2_cw_bit = encoderB;
      motor2_position--; //카운트
    }

    if(dc2_direction_type == "CCW") {
      motor2_ccw_bit = encoderB;
      motor2_position++; //카운트
    }
    
    motor2_direction = 1; // 모터 회전 방향


  } else {
    if(dc2_direction_type == "CW") {
      motor2_cw_bit = encoderB;
      motor2_position--; //카운트
    }

    if(dc2_direction_type == "CCW") {
      motor2_ccw_bit = encoderB;
      motor2_position++; //카운트
    }
    
    motor2_direction = 1; // 모터 회전 방향

    if(m2_manual == 1) {

    } else {

      // 거리측정 이동/정지
      if(dc2_direction_type == "CCW") {
        if(motor2_position > m2_target) {
          M2_stop();
          Serial.print("ISR STOP CCW >> motor2_position = ");
          Serial.print(motor2_position);
          Serial.print(",target = ");
          Serial.println(m2_target);
        }
      }

      if(dc2_direction_type == "CW") {
        if(motor2_position < m2_target) {
          M2_stop();
          Serial.print("ISR STOP CW>> motor2_position = ");
          Serial.print(motor2_position);
          Serial.print(",target = ");
          Serial.println(m2_target);
        }
      }
        
    }
  }
}


int btn1_status = 0;
void button1_press() {
  int temp = digitalRead(BTN1);
  btn1_status = 1;
  Serial.println("btn1 pressed");
}

int btn2_status = 0;
void button2_press() {
  int temp = digitalRead(BTN2);
  btn2_status = 1;
  Serial.println("btn2 pressed");
}

int btn3_status = 0;
void button3_press() {
  int temp = digitalRead(BTN3);
  btn3_status = 1;
  Serial.println("btn3 pressed");
}

int btn4_status = 0;
void button4_press() {
  int temp = digitalRead(BTN4);
  btn4_status = 1;
  Serial.println("btn4 pressed");
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



char mode;
void loop() {

/*
  for(int i=0; i<=180;i+=2) {
    servo_motor4.write(i);
    delay(5);
  }

  for(int i=180; i>0;i-=2) {
    servo_motor4.write(i);
    delay(5);
  }
*/


  /*
  builtin_led_blank(500); // 0.5초 깜빡임

  if(btn1_status == 1) {
    M1_CW_step();
  }

  if(btn2_status == 1) {
    M1_CCW_step();
  }

  if(btn3_status == 1) {
    M2_CW_step();
  }

  if(btn4_status == 1) {
    M2_CCW_step();
  }

  char cmd; // 지역변수, loop 함수에서만 사용 가능
  
  if(Serial.available()){
    cmd = Serial.read();
    parser(cmd);
  }
  */
  
  if(wire_input.length()>0){
    Serial.print("M>");
    Serial.println(wire_input);

    command_parser(wire_input);  
    wire_input = "";
  }

  //char cmd; // 지역변수, loop 함수에서만 사용 가능
  if(Serial.available()>0){
    String serial_input = Serial.readStringUntil('\n'); //엔터까지 입력받기
    Serial.println("> INPUT : " + serial_input);
    command_parser(serial_input);  
  }


  // builtin_led_blank(500); // 0.5초 깜빡임
  time_led_blank(time_blank_delay[12], LED_BUILTIN);
  //time_beep_blank(500, 12);

  // 타임딜레이 5초간격 초기화
  if(timedelay > 5000) {
    timedelay = 0; //
  }
  timedelay++; delay(1);

}


// 명령어 분석
void command_parser(String input) {


  //문자의 첫번째 위치부터 다음 토큰까지 읽어낸다
  int space_idx = input.indexOf("="); 

  //첫번째 인자의 경우, 공백을 만나지못하면 인자 딱 하나만 입력한 경우이므로 처음부터 끝까지 읽고,
  //공백을 만나면 그 공백 전까지 짤라서 읽어내면 됨. (인자가 2개 이상인 경우)
  String cmd = space_idx == -1 ? input.substring(0, input.length()) : input.substring(0, space_idx);

  if(space_idx == -1) {
    cmd = input.substring(0, input.length());
    command(cmd, ""); // 명령어 분석
  } else {
    command(cmd, input.substring(space_idx+1, input.length()) );
  }
}

void time_led_blank(int time, int pin) {
  if(timedelay % time == 0) {
    if(time_led_toggle == 0) {
      time_led_toggle = 1;
    } else {
      time_led_toggle = 0;
    }
    digitalWrite(pin, time_led_toggle);    
  }
}

void time_beep_blank(int time, int pin) {
  if(timedelay % time == 0) {
    if(time_beep_toggle == 0) {
      time_beep_toggle = 1;
    } else {
      time_beep_toggle = 0;
    }
    digitalWrite(pin, time_beep_toggle);    
  }
}




int command(String cmd, String str) {
  int module_idx = 0;

  Serial.println("command = " + cmd);

  // command prefix check
  // 커멘드에 번호가 있는지?
  //문자의 첫번째 위치부터 다음 토큰까지 읽어낸다
  int space_idx = cmd.indexOf(":");
  if(space_idx!= -1) {
    module_idx = cmd.substring(0, space_idx).toInt();;
    Serial.print("module = ");
    Serial.print(module_idx);

    cmd = cmd.substring(space_idx+1, cmd.length());
    Serial.print(", command = ");
    Serial.println(cmd);

    Wire.write('=');

    Wire.beginTransmission(module_idx);
    for(int n=0; n<str.length(); n++) {
      Wire.write(str.charAt(n));
    }

    for(int n=0; n<cmd.length(); n++) {
      Wire.write(cmd.charAt(n));
    }  
    Wire.endTransmission();
    
    return 0; //함수 중단
  } 
  

  
  // local 커멘드

  if (cmd == "beep") {
    parser_beep(str);
  }

  if (cmd == "led") {
    parser_led(str);
  }

  if (cmd == "slave") {
    int num = str.toInt();
    EEPROM.write(0,num);
  
    slave_num = num;
    Wire.begin(slave_num); 

    beep(500);
    Serial.print("I2C Slave num = ");
    Serial.println(num);
  }


  if (cmd == "motor1_target") {
    int num = str.toInt();
    m1_target = num;

    Serial.print("motor1 position=");
    Serial.print(motor1_position);
    Serial.print(", m1 target=");
    Serial.println(m1_target);
  }

  if (cmd == "motor1") {
    if(str == "move") {
      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      if(motor1_inited == 1) {
        Serial.print("motor1 move ");
        Serial.print(motor1_position);
        Serial.print(" to ");
        Serial.println(m1_target);

        M1_Move();

      } else {
        Serial.println("motor1 is not initialization!");
      }
    }    
  }

  if (cmd == "motor1_speed") {
    int num = str.toInt();
    Serial.print("motor1 speed");
    Serial.println(num);
  }

  if (cmd == "motor1_max") {
    Serial.print("motor1 max=");
    Serial.println(motor1_max);
  }

  if (cmd == "motor1_position") {
    Serial.print("motor1 position=");
    Serial.println(motor1_position);
  }

  // 모터 수동제어
  if (cmd == "motor1") {
    if(str == "ccw") {
      Serial.println("motor1 ccw");
      m1_manual = 1;
      M1_CCW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor1") {
    if(str == "stop") {
      Serial.println("motor1 stop");
      m1_manual = 1;
      //noInterrupts();
      M1_stop();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor1") {
    if(str == "cw") {
      Serial.println("motor1 cw");
      m1_manual = 1;
      //noInterrupts();
      M1_CW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }
  }

  if (cmd == "motor1") {
    if(str == "init") {
      Serial.println("motor1 init");
      m1_manual = 0;
      motor_dc1_init();
    }    
  }

  /* DC Motor2 */
  if (cmd == "motor2_speed") {
    int num = str.toInt();
    Serial.print("motor2 speed");
    Serial.println(num);
  }

  if (cmd == "motor2_max") {
    Serial.print("motor2 max=");
    Serial.println(motor2_max);
  }

  if (cmd == "motor2_position") {
    Serial.print("motor2 position=");
    Serial.println(motor2_position);
  }

  if (cmd == "motor2") {
    if(str == "ccw") {
      Serial.println("motor2 ccw");
      m2_manual = 1;
      M2_CCW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor2") {
    if(str == "stop") {
      Serial.println("motor2 stop");
      m2_manual = 1;
      //noInterrupts();
      M2_stop();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor2") {
    if(str == "cw") {
      Serial.println("motor2 cw");
      m2_manual = 1;
      //noInterrupts();
      M2_CW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor1_init") {
    Serial.println("motor1 init");
    m1_manual = 0;
    motor_dc2_init();
  }


  // Motor1, Motor2 전/후/좌/우 제어
  if (cmd == "motor") {
    if(str == "forward") {
      Serial.println("motor forward");
      m1_manual = 1;
      m2_manual = 1;
      M1_CCW();
      M2_CCW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor") {
    if(str == "stop") {
      Serial.println("motor stop");
      m1_manual = 1;
      m2_manual = 1;
      M1_stop();
      M2_stop();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor") {
    if(str == "back") {
      Serial.println("motor back");
      m1_manual = 1;
      m2_manual = 1;
      M1_CW();
      M2_CW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor") {
    if(str == "left") {
      Serial.println("motor left");
      m1_manual = 1;
      m2_manual = 1;
      M1_CCW();
      M2_CW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  if (cmd == "motor") {
    if(str == "right") {
      Serial.println("motor right");
      m1_manual = 1;
      m2_manual = 1;
      M1_CW();
      M2_CCW();

      // 수동제어를 한 경우에는, 모터 재초기화가 필요함
      motor1_inited = 0;
    }    
  }

  // PWM 서보모터
  if (cmd == "servo4") {
    int position = str.toInt();  
    
    if(server4_angle <= position) {
      for(int i=server4_angle; i<=position;i+=2) {
        servo_motor4.write(i);
        delay(5);
      }
    } else {
      for(int i=server4_angle; i>position;i-=2) {
        servo_motor4.write(i);
        delay(5);
      }
    }

    server4_angle = position;
    Serial.print("servo4 angle = ");
    Serial.println(server4_angle);
  } 



  if (cmd == "servo5") {
    int position = str.toInt();  
    
    if(server5_angle <= position) {
      for(int i=server5_angle; i<=position;i+=2) {
        servo_motor5.write(i);
        delay(5);
      }
    } else {
      for(int i=server5_angle; i>position;i-=2) {
        servo_motor5.write(i);
        delay(5);
      }
    }

    server5_angle = position;
    Serial.print("servo5 angle = ");
    Serial.println(server5_angle);
  } 

  
  
  if (cmd == "servo6") {
    int position = str.toInt();  
    
    if(server6_angle <= position) {
      for(int i=server6_angle; i<=position;i+=2) {
        servo_motor6.write(i);
        delay(5);
      }
    } else {
      for(int i=server6_angle; i>position;i-=2) {
        servo_motor6.write(i);
        delay(5);
      }
    }

    server6_angle = position;
    Serial.print("servo6 angle = ");
    Serial.println(server6_angle);
  } 




}



void parser_beep(String str) {
  int time = str.toInt();
  beep(time);
}

void parser_led(String str) {
  time_blank_delay[12] = str.toInt();
}



/*
void parser(char cmd) {
  if(cmd == 10) { 
      // lf code (10)
    } else if( cmd >= 48 && cmd <= 57) {
      if(mode == 112) { //pwm set
        m1_speed = m1_speed * 10 + (cmd - 48); 
        m2_speed = m1_speed;
        Serial.print("speed is ");
        Serial.println(m1_speed);
      } else if (mode == 101) { // e
        m1_target = m1_target * 10 + (cmd - 48); 
        Serial.print("m1 target ");
        Serial.println(m1_target);
      } else if (mode == 102) { // f
        m1_target = m1_target * 10 + (cmd - 48);
        Serial.print("m1 target ");
        Serial.println(m1_target);
      } else if (mode == 103) { // g
        m2_target = m1_target * 10 + (cmd - 48);
        Serial.print("m1 target ");
        Serial.println(m1_target);
      } else if (mode == 104) { // h
        m2_target = m1_target * 10 + (cmd - 48);
        Serial.print("m1 target ");
        Serial.println(m1_target);
      }
      
      Serial.print(cmd);
    } else if(cmd == 97) { //a
      // 모터1 (정회전)
      M1_stop();
      M1_CW();
    } else if(cmd == 98) { //b
      // 모터1 (역회전)
      M1_stop();
      M1_CCW();
    } else if(cmd == 99) { //c
      // 모터2 (정회전)
      M2_stop();
      M2_CW();
    } else if(cmd == 100) { //d
      // 모터2 (역회전)
      M2_stop();
      M2_CCW();
    } else if(cmd == 101) { //e
      // 지정거리 이동
      mode = cmd;
      m1_target = 0;
      Serial.println("motor1_cw_target set");
    } else if(cmd == 102) { //f
      mode = cmd;
      m1_target = 0;
      Serial.println("motor1_ccw_target set");
    } else if(cmd == 103) { //g
      mode = cmd;
      m2_target = 0;
      Serial.println("motor2_cw_target set");
    } else if(cmd == 104) { //h
      mode = cmd;
      m2_target = 0;
      Serial.println("motor2_ccw_target set");
    } else if(cmd == 115) { //s
      // 모터2 (역회전)
      M1_stop();
      M2_stop();
    } else if(cmd == 112) { //p
      // pwm 속도조절
      mode = cmd;
      m1_speed = 0;
      Serial.println("pwn speed set");
    } else if(cmd == 109) { // 이동 m(109) 값 전달 받은 경우 이동
      Serial.print("motor start : ");
      if(mode== 101) { //e : 지정거리 CW
        Serial.println("m1_cw");
        M1_CW_Move();
      }
      //motor_move(received_pos);
      //received_pos = 0;
    } else if(cmd == 100) { // d
      //Serial.print("max distance = ");
      //Serial.println(motor_stop_max);
    } else if(cmd == 99) { // c
      //Serial.print("current position = ");
      //Serial.println(motor_position);
    }
}
*/

/*
 * DC Motor Logic
*/

// ***_step = 버튼을 누르면 동작
void M1_CW_step() {
  int temp;
  M1_CW();

  while(btn1_status) {
    temp = digitalRead(BTN1);
    if(temp == 1) {
      M1_stop();
      btn1_status = 0;
    }
  }
}

void M1_CCW_step() {
  int temp;
  M1_CCW();

  while(btn2_status) {
    temp = digitalRead(BTN2);
    if(temp == 1) {
      M1_stop();
      btn2_status = 0;
    }
  }
}

void M2_CW_step() {
  int temp;
  M2_CW();

  while(btn3_status) {
    temp = digitalRead(BTN3);
    if(temp == 1) {
      M2_stop();
      btn3_status = 0;
    }
  }
}

void M2_CCW_step() {
  int temp;
  M2_CCW();

  while(btn4_status) {
    temp = digitalRead(BTN4);
    if(temp == 1) {
      M2_stop();
      btn4_status = 0;
    }
  }
}

void M1_Move() {

  if(m1_target < motor1_max) {
    if(m1_target < motor1_position) {
      Serial.println("Motor1 Move CCW...");    
      M1_CW();
    } else if(m1_target > motor1_position) {
      Serial.println("Motor1 Move CW...");
      M1_CCW();
    }
    
    Serial.print("motor1_init=");
    Serial.println(motor1_init);

    Serial.print("m1_manual=");
    Serial.println(m1_manual);

    
  }
  
}

// 모터 target으로 이동하는 시작 트리거(방아쇠)
void M1_CW_Move() {
  // 모터가 동작을 시작할때 1번만 호출
  Serial.print("m1 Max = ");
  Serial.println(motor1_max);

  Serial.print("current = ");
  Serial.print(motor1_position);
  Serial.print(", target = ");
  Serial.println(m1_target);

  if(m1_target < motor1_max) {
    // 시작트리거의 오류방지
    if(motor1_position < m1_target) {
      M1_CW();
    }
  }
   
}

void M1_CCW_Move() {
  // 모터가 동작을 시작할때 1번만 호출
  Serial.print("current = ");
  Serial.print(motor1_position);
  Serial.print(", target = ");
  Serial.println(m1_target);
  Serial.println("--------");

  // 시작트리거의 오류방지
  // 653 >  0
  if(m1_target > -100) {
    if(motor1_position > m1_target) {
      Serial.println("motor1 CCW");
      M1_CCW();
    } 
  }
}


// Motor1 정방향 회전
void M1_CW() {
  dc1_direction_type = "CW";

  // 정방향, 0값 최대속도
  digitalWrite(DC1_1, HIGH);
  int speed = 255 - m1_speed;
  analogWrite(DC1_2,speed);
}

// Motor1 역방향 회전
void M1_CCW()
{
  dc1_direction_type = "CCW";


  Serial.print("Motor1 CCW");
  Serial.print(", Speed = ");
  Serial.println(m1_speed);

  // 0, 1
  digitalWrite(DC1_1, LOW); 
  //digitalWrite(DC1_2, HIGH);
  //역방향, 255가 최대 속도
  analogWrite(DC1_2,m1_speed);
}


// Motor1 멈춤
void M1_stop()
{
  // 0,0 
  digitalWrite(DC1_1, LOW);
  digitalWrite(DC1_2, LOW);

  Serial.print(" M1_STOP >");
  /*
  Serial.print("motor1 direction = ");
  Serial.print(motor1_direction);

  Serial.print(", motor1 position = ");
  Serial.println(motor1_position);
  */
}


void M2_CW() {
  // 1, 0
  //digitalWrite(DC2_1, HIGH);
  //digitalWrite(DC2_2, LOW);

  digitalWrite(DC2_2, HIGH);
  int speed = 255 - m2_speed;
  analogWrite(DC2_1,speed);

}


void M2_CCW()
{
  Serial.print("Motor1 CCW");
  Serial.print(", Speed = ");
  Serial.println(m2_speed);

  // 0, 1
  digitalWrite(DC2_2, LOW); 
  // digitalWrite(DC2_2, HIGH);
  analogWrite(DC2_1,m2_speed);
}


void M2_stop()
{
  // 0,0 
  digitalWrite(DC2_1, LOW);
  digitalWrite(DC2_2, LOW);
}
