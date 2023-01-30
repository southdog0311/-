#include <switch.h>
#define BITS 10    // 10位元深度
#define STEPS 10   // 設定20階層變化

const byte LED = 25;
const byte UP_SW = 13;     // 調亮開關
const byte DOWN_SW = 27;  // 調暗開關
const byte CHANG_VAL = 1023 / STEPS;  // 每次調光的變化值

Switch upSW(UP_SW, LOW, true);
Switch downSW(DOWN_SW, LOW, true);

int pwmVal = 0;         // 電源輸出值
int LEDs[] = {22,23,4,3,19,18,5};    // for LED display
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

void showLedDisplay(int level){ // show led display correspond to light
  Serial.println(level);
  switch(level){
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

void lightUp() {
  if ((pwmVal + CHANG_VAL) <= 1023) {
    pwmVal += CHANG_VAL;
    //Serial.println(pwmVal);
    Serial.println("increase brightness of LED, cur_output: ");
    Serial.println(pwmVal);
    ledcWrite(0, pwmVal);  
    showLedDisplay(pwmVal/CHANG_VAL);      
  }
}

void lightDown() {
  if ((pwmVal - CHANG_VAL) >= 0) {
    pwmVal -= CHANG_VAL;
    Serial.println("decrease brightness of LED, cur_output: ");
    Serial.println(pwmVal);
    ledcWrite(0, pwmVal);
    showLedDisplay(pwmVal/CHANG_VAL);    
  }
}

void initalState(){
  // LED display zero and no light
  Serial.println("zero and no light");
  digitalWrite(LED, LOW);
  for(int i=0; i<7; i++){
    digitalWrite(LEDs[i], zero[i]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // 設定類比輸出
  analogSetAttenuation(ADC_11db);
  analogSetWidth(BITS);
  ledcSetup(0, 5000, BITS);
  ledcAttachPin(LED, 0);
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT); // setup LED display  
  initalState();
}

void loop() {  
  switch (upSW.check()) {    // "上"按鍵
    case Switch::RELEASED_FROM_PRESS:
    case Switch::PRESSING:
      lightUp();
      break;    
  }

  switch (downSW.check()) {  // "下"按鍵
    case Switch::RELEASED_FROM_PRESS:
    case Switch::PRESSING:
      lightDown();
      break;
  }  
}
