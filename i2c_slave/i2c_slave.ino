/*
 * I2c Slave 모듈 코드
 * version 0.0.1
*/
#include <Wire.h>
#include <EEPROM.h>
#include <Servo.h>

//#define BEEP_PIN 12 // 능동부저
#define BEEP_PIN 11 // 수동부저

//#define BUTTON1 8 // 스위치1
#define BUTTON1 12
//#define BUTTON2 9 // 스위치2
int BUTTON2;
int BUTTON3;
int BUTTON4;


// 서보모터(PWM)
Servo servo_motor1, servo_motor2, servo_motor3, servo_motor4, servo_motor5, servo_motor6;
int server1_angle=0;
int server2_angle=0;
int server3_angle=0;
int addr_servo[6]={6,7,8,9,10,11};
int servo_enable[6]={0,0,0,0,0,0};

int time_blank_delay[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int timedelay = 0;
int time_led_toggle = 0;
int time_beep_toggle = 0;

/**
 * 내부 LED 깜빡임 (초기값 1초)
*/
void builtin_led_blank(int time=1000) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(time);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(time);                      // wait for a second
}


void start_beep() {
  tone(BEEP_PIN,1046); // 도
  delay(200);
  noTone(BEEP_PIN);

  tone(BEEP_PIN,1046); // 도
  delay(200);
  noTone(BEEP_PIN);
}

void ready_beep() {
  /*
  tone(BEEP,523); // 도
  delay(500);
  
  
  tone(BEEP,587); // 레
  delay(500);
  */
  
  tone(BEEP_PIN,660); // 미
  delay(100);
  
  
  /*
  tone(BEEP,698); // 파
  delay(500);
  */
  
  tone(BEEP_PIN,784); // 솔
  delay(300);
  

  /*
  tone(BEEP,880); // 라
  delay(500);
  */
  
  tone(BEEP_PIN,988); // 시
  delay(500);
  
  /*
  tone(BEEP,1046); // 도
  delay(500);
  */

  noTone(BEEP_PIN);
 
}

void beep(int time)
{
  // 초기화2 : 비프 소리
  digitalWrite(BEEP_PIN, HIGH);
  delay(time);
  digitalWrite(BEEP_PIN, LOW);
  delay(time);
}

int status = 0;
int slave_num;

void setup() {
  int btn1, btn2, btn3, btn4;

  // 초기화: 시리얼
  Serial.begin(2000000);          // start serial for output

  // 초기화1 : 내부 LED, Beep
  pinMode(LED_BUILTIN, OUTPUT);
  time_blank_delay[12] = 500;

  pinMode(BEEP_PIN,OUTPUT); // 수동부저 출력 11핀
  start_beep();


  // 버튼 초기화
  pinMode(BUTTON1, INPUT);
  btn1 = digitalRead(BUTTON1);


  BUTTON2 = EEPROM.read(3);
  if(BUTTON2<0 || BUTTON2>19) {
    BUTTON2 = 9; // 초기화
  }
  pinMode(BUTTON2, INPUT);
  btn2 = digitalRead(BUTTON2);

  BUTTON3 = EEPROM.read(4);
  if(BUTTON3<0 || BUTTON3>19) {
    BUTTON3 = 10; // 초기화
  }
  pinMode(BUTTON3, INPUT);
  btn3 = digitalRead(BUTTON3);

  BUTTON4 = EEPROM.read(5);
  if(BUTTON4<0 || BUTTON4>19) {
    BUTTON3 = 11; // 초기화
  }
  pinMode(BUTTON4, INPUT);
  btn4 = digitalRead(BUTTON4);
  

  // 초기화: I2C 설정
  Serial.print("button1=");
  Serial.println(btn1);
  Serial.print("button2=");
  Serial.println(btn2);
  if(btn1 == 0 && btn2 == 0) {
    status = 1; // I2c 노드설정
    slave_num = 127; // 임시설정
  } else {
    slave_num = EEPROM.read(0);
    //Serial.println(slave_num);
  }

  // 강제지정
  // 1:servo1=0
  // 1:servo1=180
  slave_num = 1; 

  if(slave_num>0 && slave_num <=127) {
    Wire.begin(slave_num);  
    Wire.onReceive(receiveEvent); // register event
    Wire.onRequest(requestEvent); // register event
  }

  Serial.println("servo1 init");
  servo_motor1.attach(3);
  servo_motor1.write(0);
  delay(20);

  Serial.println("servo2 init");
  servo_motor2.attach(5);
  servo_motor2.write(0);
  delay(20);

  Serial.println("servo3 init");
  servo_motor3.attach(6);
  servo_motor3.write(0);
  delay(20);

  /*
  // 서보모터
  if(status == 1) {
    //초기화
    for(int i=0;i<6;i++) {
      EEPROM.write(addr_servo[i],0);
    }
  } else {
    for(int i=0;i<6;i++) {
      servo_enable[i] = EEPROM.read(addr_servo[i]);
    }

    if(servo_enable[0]) {
      Serial.println("servo1 init");
      servo_motor1.attach(3);
      servo_motor1.write(0);
      delay(20);
    }

    if(servo_enable[1]) {
      servo_motor2.attach(5);
      servo_motor2.write(0);
      delay(20);
    }  

    if(servo_enable[2]) {
      servo_motor3.attach(6);
      servo_motor3.write(0);
      delay(20);
    } 
  }
  */

  Serial.println("Slave Ready");
  Serial.print("Slave_Num = "); Serial.println(slave_num);
}

String wire_input;

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

void loop() {
  /*
  for(int position=0; position<180;position+=2) {
    servo_motor1.write(position);
    delay(20);
  }

  for(int position=180; position>=0;position-=2) {
    servo_motor1.write(position);
    delay(20);
  }*/
 

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
  
} /* --- loop end ---*/


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

  /* 서보모터 명령어 */
  if (cmd == "servo1") {
    int position = str.toInt();  
    
    if(server1_angle <= position) {
      for(int i=server1_angle; i<=position;i+=2) {
        servo_motor1.write(i);
        delay(5);
      }
    } else {
      for(int i=server1_angle; i>position;i-=2) {
        servo_motor1.write(i);
        delay(5);
      }
    }

    server1_angle = position;
    Serial.print("servo1 angle = ");
    Serial.println(server1_angle);
  }
  


  if (cmd == "servo2") {
    int position = str.toInt();  
    
    if(server2_angle <= position) {
      for(int i=server2_angle; i<=position;i+=2) {
        servo_motor2.write(i);
        delay(5);
      }
    } else {
      for(int i=server2_angle; i>position;i-=2) {
        servo_motor2.write(i);
        delay(5);
      }
    }

    server2_angle = position;
    Serial.print("servo2 angle = ");
    Serial.println(server2_angle);
  } 

  if (cmd == "servo3") {
    int position = str.toInt();  
    
    if(server3_angle <= position) {
      for(int i=server3_angle; i<=position;i+=2) {
        servo_motor3.write(i);
        delay(5);
      }
    } else {
      for(int i=server3_angle; i>position;i-=2) {
        servo_motor3.write(i);
        delay(5);
      }
    }

    server3_angle = position;
    Serial.print("servo3 angle = ");
    Serial.println(server3_angle);
  } 
  


  







}



void parser_beep(String str) {
  int time = str.toInt();
  beep(time);
}

void parser_led(String str) {
  time_blank_delay[12] = str.toInt();
}


/* =====
 * command
   ===== */



