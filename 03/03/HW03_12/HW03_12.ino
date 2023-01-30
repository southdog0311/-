#include <ESP32Servo.h> //請先安裝ESP32Servo程式庫
Servo myServo; //建立一個伺服馬達物件
#define LED1 2
#define LED2 16
#define RED_BTN 18
#define BLUE_BTN 32
#define GREEN_BTN 33
#define BITS 10
#define BUZZER_PIN 22  // 蜂鳴器接在腳22
const int holdTime = 10000;
int pos = 0, pressTime=0;

void resetLight(){
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
}

void alarm(){ // shine two LEDs 
  pressTime = millis(); // cur times
  while( (millis() - pressTime) < holdTime && digitalRead()){    
    digitalWrite(LED1, HIGH);
    delay(50);
    digitalWrite(LED2, LOW);
    delay(50);
    digitalWrite(LED1, LOW);
    delay(50);
    digitalWrite(LED2, HIGH);
    delay(50);
  }
  resetLight();
}

void alarmForever(){ // shine two LEDs 
  pressTime = millis(); // cur times
  while(digitalRead(GREEN_BTN)){    
    digitalWrite(LED1, HIGH);
    delay(50);
    digitalWrite(LED2, LOW);
    delay(50);
    digitalWrite(LED1, LOW);
    delay(50);
    digitalWrite(LED2, HIGH);
    delay(50);
  }
  resetLight();
}

void setup(){
  ledcSetup(1, 20000, BITS);
  ledcAttachPin(BUZZER_PIN, 1);
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(BLUE_BTN, INPUT_PULLUP);
  pinMode(GREEN_BTN, INPUT_PULLUP);
  myServo.attach(17);
}
void loop(){
  if(digitalRead(RED_BTN)){
    Serial.println("高電平");    
    delay(1000);
  }
  else{
     Serial.println("低電平");
     // 0到180旋轉舵機，每次延時15毫秒
      for(pos = 0; pos < 90; pos += 1) { 
         myServo.write(pos); 
         delay(15); 
      } 
     Serial.println("打開柵欄"); 
      ledcWriteTone(1, 400);  //蜂鳴器鳴叫
      alarm();  
      ledcWriteTone(1, 0);  //蜂鳴器停止   
      myServo.attach(17);   
      // 180到0旋轉舵機，每次延時15毫秒 
      for(pos = 90; pos>=1; pos-=1)
      {                               
        myServo.write(pos);
        delay(15);
      }
      Serial.println("關閉柵欄");
  }
  if(digitalRead(BLUE_BTN)){
    Serial.println("紅色按鈕高電平"); 
    delay(1000);
  }
  else{
    Serial.println("低電平");
    // 0到180旋轉舵機，每次延時15毫秒
      for(pos = 0; pos < 90; pos += 1) { 
         myServo.write(pos); 
         delay(15); 
      } 
     Serial.println("打開柵欄");     
     alarmForever();  
  }
  if(digitalRead(GREEN_BTN)){
    Serial.println("綠色按鈕高電平"); 
    delay(1000);
  }
  else{
    Serial.println("低電平");
    // 0到180旋轉舵機，每次延時15毫秒
      for(pos = 90; pos > 1; pos -= 1) { 
         myServo.write(pos); 
         delay(15); 
      } 
     Serial.println("關閉柵欄");     
  }
}
