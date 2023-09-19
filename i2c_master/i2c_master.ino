/*
 * I2C 모듈 마스터
 * version 0.0.1
 * by hojin
*/
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h> // 라이브러리 설치
#include <SoftwareSerial.h>

#define DEBUG 1 

#define BEEP_PIN 11 // 능동부저
#define BEEP 11 // 수동부저


#define BUTTON1 8 
#define BUTTON2 9
#define BUTTON3 10




#define BT_RXD 7 // 블루투스모듈 TX연결
#define BT_TXD 6 // 블루투스모듈 RX연결

// 0 : ready
// 1 : setup
int status = 0;
int timedelay = 0;



int address = 0;


int time_blank_delay[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// i2c 기기 번호 0x27
LiquidCrystal_I2C lcd(0x27,16,2);

// 블루투스 소프트시리얼
SoftwareSerial bluetooth(BT_RXD, BT_TXD);


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

void lcd_init() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
}

String ProductName;
int addr_title_enable = 128;
int product_name_addr = 256;

void product_name_read() {
  char c;
  if(EEPROM.read(addr_title_enable) == 1) {
    
    for(int a=product_name_addr, i=0; a<16; a++,i++) {
      c = EEPROM.read(a);
      if( c > 20 && c <= 126 ) {
        ProductName += c; 
      } else {
        ProductName += '_'; 
      }
    }
  } else {
    ProductName = "I2C-Master";
  }
}

void start_beep() {
  tone(BEEP,1046); // 도
  delay(200);
  noTone(BEEP);

  tone(BEEP,1046); // 도
  delay(200);
  noTone(BEEP);
}

void ready_beep() {
  /*
  tone(BEEP,523); // 도
  delay(500);
  
  
  tone(BEEP,587); // 레
  delay(500);
  */
  
  tone(BEEP,660); // 미
  delay(100);
  
  
  /*
  tone(BEEP,698); // 파
  delay(500);
  */
  
  tone(BEEP,784); // 솔
  delay(300);
  

  /*
  tone(BEEP,880); // 라
  delay(500);
  */
  
  tone(BEEP,988); // 시
  delay(500);
  
  /*
  tone(BEEP,1046); // 도
  delay(500);
  */

  noTone(BEEP);
 
}

void setup() {
  // 초기화0. EEPROM
  // - Arduino Uno:         1 kB EEPROM storage.

  // 초기화1 : 내부 LED, Beep
  pinMode(LED_BUILTIN, OUTPUT);
  time_blank_delay[12] = 500;

  // 초기화2. 전원 입력시 삑 (1번)
  /*
  pinMode(BEEP_PIN, OUTPUT);
  beep(50); // 삐빅
  */

  pinMode(BEEP,OUTPUT); // 수동부저 출력 11핀
  start_beep();

  

  // 초기화4. 시리얼 초기화
  Serial.begin(2000000);
  beep(100); // 삐빅

  product_name_read();
  Serial.print(ProductName); 

  // 초기화3. LCD 초기화
  lcd_init();
  lcd.setCursor(0,0);
  lcd.print(ProductName);


  // 초기화7. 버튼 
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);



  /*
  if(digitalRead(BUTTON1) == 0) {
    beep(200); // 삐빅
    status = 1; // setup 상태

    lcd.setCursor(0,1);
    lcd.print("Setup mode");
    //lcd.blink();

    Serial.println(" = Setup Mode!");
  } else {
    
    lcd.setCursor(0,1);
    lcd.print("ready!");
    Serial.println(" = Ready!");
  }
  */

  lcd.setCursor(0,1);
  lcd.print("ready!");
  Serial.println(" = Ready!");

  
  // 초기화5. I2C
  Wire.begin(); // i2c 통신

  // 초기화6. 블루투스
  bluetooth.begin(9600);

  ready_beep();

}




// 모듈로 값을 전달합니다.
void commandModule(int module, int value)
{
  Wire.beginTransmission(module);
  Wire.write(value);
  Wire.endTransmission();
}

/*
  버튼 검출 로직
*/
int button1_press = 0;
int button1_time_count = 0;
int button1_press_check() {
  int count;
  if(digitalRead(BUTTON1) == 0) {
    if(button1_press == 0) {
      button1_press = timedelay;
    }
    button1_time_count++;
  }
  
  if(digitalRead(BUTTON1) == 1) {
    if(button1_press > 0) {        
      button1_press = 0;
      Serial.print("button1 press = ");      
      Serial.println( button1_time_count );
      count = button1_time_count;
      button1_time_count=0;
      return count;
    }
  }

  return 0;
}

int button2_press = 0;
int button2_time_count = 0;
int button2_press_check() {
  int count;
  if(digitalRead(BUTTON2) == 0) {
    if(button2_press == 0) {
      button2_press = timedelay;
    }
    button2_time_count++;
  }
  
  if(digitalRead(BUTTON2) == 1) {
    if(button2_press > 0) {        
      button2_press = 0;
      Serial.print("button2 press = ");      
      Serial.println( button2_time_count );
      count = button2_time_count;
      button2_time_count=0;
      return count;
    }
  }

  return 0;
}

int button3_press = 0;
int button3_time_count = 0;
int button3_press_check() {
  int count;
  if(digitalRead(BUTTON3) == 0) {
    if(button3_press == 0) {
      button3_press = timedelay;
    }
    button3_time_count++;
  }
  
  if(digitalRead(BUTTON3) == 1) {
    if(button3_press > 0) {        
      button3_press = 0;
      Serial.print("button3 press = ");      
      Serial.println( button3_time_count );
      count = button3_time_count;
      button3_time_count=0;
      return count;
    }
  }

  return 0;
}

String input;
String bluetooth_input;

int time_led_toggle = 0;
int time_beep_toggle = 0;

int mode_count = 0;

void loop() {
  // 설정모드
  if(status == 1) {

  }

  // 블루투스 통신
  if(bluetooth.available()){
    bluetooth_input = bluetooth.readStringUntil('\n'); //엔터까지 입력받기
    Serial.print("+< ");
    Serial.println(bluetooth_input);
    command_parser(bluetooth_input);
  }
  
  // 시리얼 통신
  if(Serial.available()>0){
    String serial_input = Serial.readStringUntil('\n'); //엔터까지 입력받기
    Serial.println("> " + serial_input);
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


/*
  I2C 통신
*/

void wire_request(int id, int len) {
    Wire.requestFrom(id, len);    // request 6 bytes from slave device #8

  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }
}

/*
  명령어 처리
*/
// 명령어 분석
void command_parser(String input) {
  if(input.charAt(0) == '?') {
    help();
    return;
  } 

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

void help() {
  Serial.println("----- ----- ----- ----- -----");
  Serial.println("+명령: 블루투스AT 명령전송");
  Serial.println("i번호: i2c select");
  Serial.println("lcd=문자열: LCD출력");
  Serial.println("beep=시간: 부저출력");
  Serial.println("----- ----- ----- ----- -----");
}


int command(String cmd, String str) {
  int module_idx = 0;

  Serial.println("command = " + cmd);

  // command prefix check
  // 커멘드에 번호가 있는지?
  //문자의 첫번째 위치부터 다음 토큰까지 읽어낸다
  int space_idx = cmd.indexOf(":");
  if(space_idx!= -1) {
    // 모듈 번호 구분
    module_idx = cmd.substring(0, space_idx).toInt();
    Serial.print("Module = ");
    Serial.print(module_idx);

    // 지정한 I2C 모듈 선택
    Wire.beginTransmission(module_idx);

    // 명령어를 전송합니다.
    Serial.print(", Command = ");
    for(int n=(space_idx+1); n<cmd.length(); n++) {
      Wire.write(cmd.charAt(n));
      Serial.print(cmd.charAt(n));
    } 

    // 2차 매개변수가 있는 경우 데이터 추가 전송
    if(str.length()>0) {
      // 구분기호 전송
      Wire.write('=');
      //Serial.println("");

      // 파라미터 값을 추가 전송합니다.
      //Serial.print("Wrie value = ");
      for(int n=0; n<str.length(); n++) {
        Wire.write(str.charAt(n));
        Serial.print(str.charAt(n));
      }

      Serial.println("...");
    }
     
    Wire.endTransmission();
    
    return 0; //함수 중단
  } 
  
  // 블루투스 코멘드
  // +시작하는 명령 : 
  if(cmd.charAt(0) == '+') {
    // +를 제외한 명령
    // 블루투스 명령 전송
    for(int n=1; n<cmd.length(); n++) {
      bluetooth.write(cmd.charAt(n));
    }
  }
  
  // local 커멘드
  if (cmd == "i2c-scan") {
    parser_i2c_scan(str);
  }

  if (cmd == "lcd") {
    parser_lcd(str);
  }

  if (cmd == "beep") {
    parser_beep(str);
  }

  if (cmd == "led") {
    parser_led(str);
  }

  if (cmd == "title") {
    int a = product_name_addr;
    for(int i=0; i<str.length(); a++,i++) {
      EEPROM.write(a, str.charAt(i));
      //Serial.print(" ");
      //Serial.print(EEPROM.read(a));
    }
    if(str.length() < 16) {
      for(int i=str.length(); i<16; a++,i++) {
        EEPROM.write(a, 20); // 공백  
      }
    }
    //Serial.println("");

    for(int i=str.length(); i<16;i++) {
      str += ' ';
    }

    lcd.setCursor(0,0);
    lcd.print(str);
  }

}

// LCD 출력
// 첫번째 줄과 두번째 줄은 `/`로 구분함
void parser_lcd(String str) {
  lcd.clear();

  // 첫번째줄 읽기
  int space_idx = str.indexOf("/"); //문자의 첫번째 위치부터 다음 공백까지 읽어낸다

  //첫번째 인자의 경우, 공백을 만나지못하면 인자 딱 하나만 입력한 경우이므로 처음부터 끝까지 읽고,
  //공백을 만나면 그 공백 전까지 짤라서 읽어내면 됨. (인자가 2개 이상인 경우)
  String arg0;
  if(space_idx == -1) {
    arg0 = str.substring(0, str.length()); // '/' 를 검출하지 못한경우
  } else {
    arg0 = str.substring(0, space_idx);
  }

  // 첫번째줄 출력
  lcd.setCursor(0,0);
  lcd.print(arg0);
  // Serial.println(arg0);

  // 다음 공백 위치 찾는 코드
  // Serial.println(space_idx);
  // '/'로 시작해서, 첫번째줄을 생략하는 경우
  String arg1 = "";
  if(space_idx == -1) {
    arg1="";
  } else {
    arg1 = str.substring(space_idx + 1, str.length());
  }
  
  // 2번째줄 출력
  lcd.setCursor(0,1);
  Serial.println(arg1);
  lcd.print(arg1);
}

// I2c 스케닝
void parser_i2c_scan(String str) {
  byte Err, adr;
  int N_Device;

  Serial.println("Scanning...");
  N_Device = 0;

  for(adr=1;adr<127; adr++){
    Wire.beginTransmission(adr);
    Err = Wire.endTransmission();
    
    // 전송성공
    if(Err == 0) {
      Serial.print("0x");
      Serial.print(adr, HEX);
      Serial.print(" => ");
      Serial.println("I2C device found at address");
      N_Device++;
    }

    if(N_Device == 0) {
      Serial.println("No I2C Device found");
    } else {
      Serial.println("Done");
    }
  }
}

// 비프소리 출력
void parser_beep(String str) {
  int time = str.toInt();
  beep(time);
}

// 내부 LED 깜빡임 속도 변경
void parser_led(String str) {
  time_blank_delay[12] = str.toInt();
}


