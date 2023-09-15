/*
 * I2C 모듈 마스터
 * version 0.0.1
 * by hojin
*/
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h> // 라이브러리 설치
#include <SoftwareSerial.h>



#define BEEP_PIN 12 // 부저

#define BUTTON1 8 
#define BUTTON2 9
#define BUTTON3 10

#define BT_RXD 7 // 블루투스모듈 TX연결
#define BT_TXD 6 // 블루투스모듈 RX연결

int i2c_module = 0;

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
int product_name_addr = 0;

void product_name_read() {
  char c;
  for(int a=product_name_addr, i=0; a<16; a++,i++) {
    c = EEPROM.read(a);
    if( c >= 20 && c <= 126 ) {
      ProductName += c; 
    }
  }

  //Serial.println(ProductName); 
}

void setup() {
  // 초기화0. EEPROM
  // - Arduino Uno:         1 kB EEPROM storage.
  //EEPROM.write(address, 100);
  // 
  



  


  // 초기화1 : 내부 LED, Beep
  pinMode(LED_BUILTIN, OUTPUT);
  time_blank_delay[12] = 500;

  // 초기화2. 전원 입력시 삑 (1번)
  pinMode(BEEP_PIN, OUTPUT);
  beep(200); // 삐빅

  // 초기화3. LCD 초기화
  lcd_init();
  

  // 초기화4. 시리얼 초기화
  Serial.begin(2000000);

  product_name_read();
  if(ProductName == "") {
    ProductName = "I2C-Master";
  }
  
  // 초기화5. I2C
  Wire.begin(); // i2c 통신

  // 초기화6. 블루투스
  bluetooth.begin(9600);


  // 초기화7. 버튼 
  pinMode(BUTTON1, INPUT);
  //attachPCINT( digitalPinToPCINT(BUTTON1), button1_press, FALLING);
  pinMode(BUTTON2, INPUT);
  //attachPCINT( digitalPinToPCINT(BUTTON2), button2_press, FALLING);
  pinMode(BUTTON3, INPUT);
  //attachPCINT( digitalPinToPCINT(BUTTON3), button3_press, FALLING);



  if(digitalRead(BUTTON1) == 0) {
    Serial.println("Setup Mode!");
    beep(200); // 삐빅

    lcd.setCursor(0,0);
    lcd.print("Setup mode");

  } else {
    Serial.println("Master Ready!");

    lcd.setCursor(0,0);
    lcd.print(ProductName);
    lcd.setCursor(0,1);
    lcd.print("ready!");
  }


}


void button1_press() {
  Serial.println("button1 press");
}

void button2_press() {
  Serial.println("button2 press");
}

void button3_press() {
  Serial.println("button3 press");
}

// 모듈로 값을 전달합니다.
void commandModule(int module, int value)
{
  Wire.beginTransmission(module);
  Wire.write(value);
  Wire.endTransmission();
}



String input;
String bluetooth_input;
int timedelay = 0;
int time_led_toggle = 0;
int time_beep_toggle = 0;



void loop() {
  
  if(bluetooth.available()){
    bluetooth_input = bluetooth.readStringUntil('\n'); //엔터까지 입력받기
    Serial.print("+< ");
    Serial.println(bluetooth_input);
  }
  
  //char cmd; // 지역변수, loop 함수에서만 사용 가능
  if(Serial.available()>0){
    input = Serial.readStringUntil('\n'); //엔터까지 입력받기
    Serial.println("> INPUT : " + input);

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
  


  // builtin_led_blank(500); // 0.5초 깜빡임
  time_led_blank(time_blank_delay[12], LED_BUILTIN);
  //time_beep_blank(500, 12);

  // 타임딜레이 5초간격 초기화
  if(timedelay > 5000) {
    timedelay = 0; //
  }
  timedelay++; delay(1);
  
} /* --- loop end ---*/

void help() {
  Serial.println("----- ----- ----- ----- -----");
  Serial.println("+명령: 블루투스AT 명령전송");
  Serial.println("i번호: i2c select");
  Serial.println("lcd=문자열: LCD출력");
  Serial.println("beep=시간: 부저출력");
  Serial.println("----- ----- ----- ----- -----");
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
  
  // +시작하는 명령 : 블루투스 코멘드
  if(cmd.charAt(0) == '+') {
    // +를 제외한 명령
    // 블루투스 명령 전송
    for(int n=1; n<cmd.length(); n++) {
      bluetooth.write(cmd.charAt(n));
    }
  }
  
  // local 커멘드

  if (cmd == "i2c") {
    parser_i2c(str);
  }

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


void parser_lcd(String str) {
  Serial.println("---");
  Serial.println(str);
  int space_idx = str.indexOf("/"); //문자의 첫번째 위치부터 다음 공백까지 읽어낸다

  lcd.clear();

  //첫번째 인자의 경우, 공백을 만나지못하면 인자 딱 하나만 입력한 경우이므로 처음부터 끝까지 읽고,
  //공백을 만나면 그 공백 전까지 짤라서 읽어내면 됨. (인자가 2개 이상인 경우)
  String arg0;
  if(space_idx == -1) {
    arg0 = str.substring(0, str.length());
  } else {
    arg0 = str.substring(0, space_idx);
  }
  //arg0 = space_idx == -1 ? input.substring(0, str.length()) : str.substring(0, space_idx);

  lcd.setCursor(0,0);
  Serial.println(arg0);
  lcd.print(arg0);

  Serial.println(space_idx);
  // 다음 공백 위치 찾는 코드
  //int backup_space = space_idx;                  //공백 위치를 백업해둠
  //space_idx = str.indexOf(" ", space_idx + 1); //다음 공백 위치를 찾는다.
  //String arg1 = space_idx == -1 ? str.substring(backup_space + 1, str.length()) : str.substring(backup_space + 1, space_idx);
  String arg1 = "";
  if(space_idx == -1) {
    arg1="";
  } else {
    arg1 = str.substring(space_idx + 1, str.length());
  }
  
  lcd.setCursor(0,1);
  Serial.println(arg1);
  lcd.print(arg1);
}

void parser_i2c(String str) {
  int num = str.toInt();
  i2c_module = num;
  Serial.print("select i2c module = ");
  Serial.println(num);
}

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

void parser_beep(String str) {
  int time = str.toInt();
  beep(time);
}

void parser_led(String str) {
  time_blank_delay[12] = str.toInt();
}


