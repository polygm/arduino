/*
 * I2C 모듈 마스터
 * version 0.0.1
 * by hojin
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // 라이브러리 설치

#define BEEP_PIN 12 // 부저

int i2c_module = 0;

char ProductName[] = "PolyFit";

// i2c 기기 번호 0x27
LiquidCrystal_I2C lcd(0x27,16,2);

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

void setup() {
  // 초기화1 : 내부 LED, Beep
  pinMode(LED_BUILTIN, OUTPUT);

  // 초기화2. 전원 입력시 삑 (1번)
  pinMode(BEEP_PIN, OUTPUT);
  beep(200); // 삐빅

  // 초기화3. LCD 초기화
  lcd_init();
  lcd.print(ProductName);
  lcd.setCursor(0,1);
  lcd.print("ready!");

  // 초기화4. 시리얼 초기화
  Serial.begin(115200);
  Serial.println("Master Ready!");

  // 초기화5. I2C
  Wire.begin(); // i2c 통신

}

// 모듈로 값을 전달합니다.
void commandModule(int module, int value)
{
  Wire.beginTransmission(module);
  Wire.write(value);
  Wire.endTransmission();
}

String input;

void loop() {
  builtin_led_blank(500); // 0.5초 깜빡임

  char cmd; // 지역변수, loop 함수에서만 사용 가능
  if(Serial.available()>0){
    input = Serial.readStringUntil('\n'); //엔터까지 입력받기
    Serial.println("INPUT : " + input);

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
}


void command(String cmd, String str) {
  int module_idx = 0;

  Serial.println("command = " + cmd);

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

  } else {
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

/*
char buffers[50];
int buf_index = 0;
int buf_num = 0;
void serial_buffers(char cmd) {
  if(cmd == 10) { // lf code (10)
    if(parser()) {
      // 처리할 로직이 없는 경우
      // 입력한 값을 화면 출력함
      for(int i=0; i<buf_index; i++){
        Serial.print(buffers[i]);
      }
      Serial.println("");
    }
    buf_index = 0;
    buf_num = 0;
  } else if(cmd == 20) {
    buffers[buf_index++] = cmd;
    Serial.println("space");
  } else if(cmd == '/') {
    buffers[buf_index++] = cmd;
  } else if(cmd >= 48 && cmd <= 57) {
    buf_num = buf_num * 10 + (cmd - 48);
    buffers[buf_index++] = cmd;
  } else if(cmd >= 97 && cmd <= 122) {
    buffers[buf_index++] = cmd;
  } else if(cmd == '?') {
    Serial.println("-----");
    Serial.println("i=i2c select");
    Serial.println("-----");
  }
}

int parser() {
  if(buffers[0] == 'i') {
    Serial.print("i2c select = ");
    if(buf_num>0 && buf_num<127) {
      Serial.println(buf_num);
    } else {
      Serial.println("number error");
    }

    return 0;
  } else if(buffers[0] == 'l' && buffers[1] == 'c' && buffers[2] == 'd') {
    Serial.println(buffers);

    lcd.clear();

    char str1[16]="                ", str2[16];
    int i,j;
    lcd.setCursor(0,0);
    for(i=4, j=0; i<strlen(buffers); i++, j++) {
      if(buffers[i] == '/') {
        break;
      }
      str1[j] = buffers[i];
    }
    lcd.print(str1);
    Serial.println(str1);

    lcd.setCursor(0,1);
    for(int j=0; i<strlen(buffers); i++, j++) {
      str2[j] = buffers[i];
    }
    lcd.print(str2);
    Serial.println(str2);

  }
  // 처리할 로직 없음
  return 1;
}
*/

