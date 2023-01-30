#include <U8g2lib.h>         // OLED 相關函式庫
#include <ESP32Servo.h>      // 伺服馬達
#include <switch.h>          // 按鈕開關
Servo myServo; //建立一個伺服馬達物件
#define BITS 10
#define STEPS 10 // 設定 10 階層變化瓦斯濃度等級

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
String msg;  // 顯示在螢幕的訊息字串
const byte RED_LED = 16;    // 警告燈號
const byte BLUE_LED = 26;   // 警告燈號
int threshold = 5; // 瓦斯濃度臨界值(預設值)
const int adjust_MQ2 = 33;  // 校準瓦斯感測器等級
const byte UP_SW = 13;      // 調高瓦斯濃度臨界值開關
const byte DOWN_SW = 27;    // 調低瓦斯濃度臨界值開關
int pos = 0;                // 窗戶的目前位置

Switch upSW(UP_SW, LOW, true);
Switch downSW(DOWN_SW, LOW, true);

int LEDs[] = {25,23,4,3,19,18,32};    // for LED display
// g,f,e,d,c,b,a 
int zero[] = {0, 1, 1, 1, 1, 1, 1};
int one[] = {0, 0, 0, 0, 1, 1, 0};   // LED states to display number one
int two[] = {1, 0, 1, 1, 0, 1, 1};
int three[] = {1, 0, 0, 1, 1, 1, 1};
int four[] = {1, 1, 0, 0, 1, 1, 0};
int five[] = {1, 1, 0, 1, 1, 0, 1};
int six[] = {1, 1, 1, 1, 1, 0, 1};
int seven[] = {0, 0, 0, 0, 1, 1, 1};
int eight[] = {1, 1, 1, 1, 1, 1, 1}; // LED states to display number eight
int nine[] = {1, 1, 0, 1, 1, 1, 1};

void setupU8g2(){
  u8g2.setFont(u8g2_font_unifont_t_chinese1);  // 使用 chinese1字型檔
  u8g2.setFontDirection(0);
}

void alarm(){ // shine two LEDs 
  digitalWrite(BLUE_LED, HIGH);
  delay(50);
  digitalWrite(RED_LED, LOW);
  delay(50);
  digitalWrite(BLUE_LED, LOW);
  delay(50);
  digitalWrite(RED_LED, HIGH);
  delay(50);
  digitalWrite(BLUE_LED, HIGH);
  delay(50);
  digitalWrite(RED_LED, LOW);
  delay(50);
  digitalWrite(BLUE_LED, LOW);
  delay(50);
  digitalWrite(RED_LED, HIGH);
  delay(50);
}

void displayOLED(String msg){ // 顯示訊息於 OLED 上面 
  u8g2.clearBuffer();
  u8g2.setCursor(0, 40);
  u8g2.print(msg);   
  u8g2.sendBuffer();
  u8g2.drawBox(5, 5,20, 20);  
}

void displayLED(int threshold){ // 顯示臨界值數值於 7 段式顯示器
  if(threshold > 9) threshold = 9;
  if(threshold < 0) threshold = 0;
  Serial.println("目前瓦斯濃度臨界值: " + String(threshold));
  switch(threshold){
    case 0: 
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], zero[i]);
    break;
    case 1:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], one[i]);      
    break;
    case 2:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], two[i]);
    break;
    case 3:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], three[i]);
    break;
    case 4:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], four[i]);
    break;
    case 5:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], five[i]);
    break;
    case 6:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], six[i]);
    break;
    case 7:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], seven[i]);
    break;
    case 8:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], eight[i]);
    break;
    case 9:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], nine[i]);
    break;
  }
}

void initalState(){ // 初始值所有燈
  // LED display zero and no light
  Serial.println("zero and no light");
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  displayLED(threshold);
}

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.enableUTF8Print();  // 啟用顯示UTF-8 編碼字串
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db); // 設定類筆輸入電壓上限 3.6V
  analogSetWidth(BITS);           // 取樣設成10位元
  ledcSetup(0, 5000, BITS);       // 設定 PWM，通道0、KHz、10位元
  myServo.attach(17);             // 設定馬達接腳
  myServo.write(pos); 
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT); // setup LED display  
  initalState(); // 初始化
}

void loop() {
  setupU8g2();  
  uint16_t adc = analogRead(A0); // MQ2 感測值
  adc = map(adc, 0, 255, 0, 1023); // Mapping 至 1023  值
  uint16_t mq2 = adc/adjust_MQ2; 
  Serial.printf("MQ2感測值: %u, %u\n", adc, mq2); 
  //delay(30);
  msg = "現在濃度 " + (String)(mq2);  
  displayOLED(msg);  
  switch (upSW.check()) {    // "上"調高瓦斯濃度臨界值
    case Switch::RELEASED_FROM_PRESS:
    case Switch::PRESSING:
      threshold+=1;
      displayLED(threshold);
      break;    
  }

  switch (downSW.check()) {  // "下"調低瓦斯濃度臨界值
    case Switch::RELEASED_FROM_PRESS:
    case Switch::PRESSING:
      threshold-=1;
      displayLED(threshold);
      break;
  }  
  if(mq2 >= threshold && pos<=1){ // 表示瓦斯濃度超過安全範圍
    // 0到90旋轉舵機，每次延時15毫秒
      digitalWrite(RED_LED, HIGH); // 調節燈的亮度 
      for(pos = 0; pos < 90; pos += 1) { 
         //alarm();
         myServo.write(pos); 
         delay(15); 
      } 
      alarm();
      Serial.println("窗戶已開啟");
  }
  if(mq2 <= threshold-1 && pos>=89){ // 表示瓦斯濃度為安全範圍內
    digitalWrite(RED_LED, LOW); // 調節燈的亮度 
    digitalWrite(BLUE_LED, LOW); // 調節燈的亮度 
    for(pos = 90; pos > 1; pos -= 1) { 
         myServo.write(pos); 
         delay(15); 
    }
    Serial.println("窗戶已關閉");
  }
  delay(1000);
}
